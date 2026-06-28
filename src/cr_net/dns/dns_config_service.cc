// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_net/dns/dns_config_service.h"

#include <string>

#include "cr_base/logging/logging.h"
#include "cr_base/functional/bind.h"
#include "cr_base/files/file_path.h"
#include "cr_base/location.h"
#include "cr_base/memory/ref_ptr.h"
#include "cr_base/containers/optional.h"
#include "cr_base/threading/sequence/sequence_checker.h"
#include "cr_base/time/time.h"

#include "cr_event/task/sequenced_task_runner_handle.h"

#include "cr_net/dns/dns_hosts.h"
#include "cr_net/dns/serial_worker.h"

namespace cr {
namespace net {

// static
const cr::TimeDelta DnsConfigService::kInvalidationTimeout =
    cr::TimeDelta::FromMilliseconds(150);

DnsConfigService::DnsConfigService(
    cr::FilePath::StringPieceType hosts_file_path,
    cr::Optional<cr::TimeDelta> config_change_delay)
    : watch_failed_(false),
      have_config_(false),
      have_hosts_(false),
      need_update_(false),
      last_sent_empty_(true),
      config_change_delay_(config_change_delay),
      hosts_file_path_(hosts_file_path) {
  CR_DETACH_FROM_SEQUENCE(sequence_checker_);
}

DnsConfigService::~DnsConfigService() {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  if (hosts_reader_)
    hosts_reader_->Cancel();
}

void DnsConfigService::ReadConfig(const CallbackType& callback) {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  CR_DCHECK(!callback.is_null());
  CR_DCHECK(callback_.is_null());
  callback_ = callback;
  ReadConfigNow();
  ReadHostsNow();
}

void DnsConfigService::WatchConfig(const CallbackType& callback) {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  CR_DCHECK(!callback.is_null());
  CR_DCHECK(callback_.is_null());
  callback_ = callback;
  watch_failed_ = !StartWatching();
  ReadConfigNow();
  ReadHostsNow();
}

void DnsConfigService::RefreshConfig() {
  // Overridden on supported platforms.
  CR_NOTREACHED();
}

DnsConfigService::Watcher::Watcher(DnsConfigService& service)
    : service_(&service) {}

DnsConfigService::Watcher::~Watcher() {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
}

void DnsConfigService::Watcher::OnConfigChanged(bool succeeded) {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  service_->OnConfigChanged(succeeded);
}

void DnsConfigService::Watcher::OnHostsChanged(bool succeeded) {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  service_->OnHostsChanged(succeeded);
}

void DnsConfigService::Watcher::CheckOnCorrectSequence() {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
}

DnsConfigService::HostsReader::HostsReader(
    cr::FilePath::StringPieceType hosts_file_path,
    DnsConfigService& service)
    : service_(&service), hosts_file_path_(hosts_file_path) {}

DnsConfigService::HostsReader::~HostsReader() = default;

cr::Optional<DnsHosts> DnsConfigService::HostsReader::ReadHosts() {
  ///cr::ScopedBlockingCall scoped_blocking_call(FROM_HERE,
  ///                                            base::BlockingType::MAY_BLOCK);
  CR_DCHECK(!hosts_file_path_.empty());
  DnsHosts dns_hosts;
  if (!ParseHostsFile(hosts_file_path_, &dns_hosts))
    return cr::nullopt;

  return dns_hosts;
}

bool DnsConfigService::HostsReader::AddAdditionalHostsTo(
    DnsHosts& in_out_dns_hosts) {
  // Nothing to add in base implementation.
  return true;
}

void DnsConfigService::HostsReader::DoWork() {
  hosts_ = ReadHosts();
  if (!hosts_.has_value())
    return;

  if (!AddAdditionalHostsTo(hosts_.value()))
    hosts_.reset();
}

void DnsConfigService::HostsReader::OnWorkFinished() {
  if (hosts_.has_value()) {
    service_->OnHostsRead(std::move(hosts_).value());
  } else {
    CR_LOG(Warning) << "Failed to read DnsHosts.";
  }
}

void DnsConfigService::ReadHostsNow() {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (!hosts_reader_) {
    CR_DCHECK(!hosts_file_path_.empty());
    hosts_reader_ =
        cr::MakeRefCounted<HostsReader>(hosts_file_path_.value(), *this);
  }
  hosts_reader_->WorkNow();
}

void DnsConfigService::InvalidateConfig() {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  if (!have_config_)
    return;
  have_config_ = false;
  StartTimer();
}

void DnsConfigService::InvalidateHosts() {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  if (!have_hosts_)
    return;
  have_hosts_ = false;
  StartTimer();
}

void DnsConfigService::OnConfigRead(DnsConfig config) {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  CR_DCHECK(config.IsValid());

  if (!config.EqualsIgnoreHosts(dns_config_)) {
    dns_config_.CopyIgnoreHosts(config);
    need_update_ = true;
  }

  have_config_ = true;
  if (have_hosts_ || watch_failed_)
    OnCompleteConfig();
}

void DnsConfigService::OnHostsRead(DnsHosts hosts) {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (hosts != dns_config_.hosts) {
    dns_config_.hosts = std::move(hosts);
    need_update_ = true;
  }

  have_hosts_ = true;
  if (have_config_ || watch_failed_)
    OnCompleteConfig();
}

void DnsConfigService::StartTimer() {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  if (last_sent_empty_) {
    CR_DCHECK(!timer_.IsRunning());
    return;  // No need to withdraw again.
  }
  timer_.Stop();

  // Give it a short timeout to come up with a valid config. Otherwise withdraw
  // the config from the receiver. The goal is to avoid perceivable network
  // outage (when using the wrong config) but at the same time avoid
  // unnecessary Job aborts in HostResolverImpl. The signals come from multiple
  // sources so it might receive multiple events during a config change.
  timer_.Start(CR_FROM_HERE, kInvalidationTimeout, this,
               &DnsConfigService::OnTimeout);
}

void DnsConfigService::OnTimeout() {
  CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  CR_DCHECK(!last_sent_empty_);
  // Indicate that even if there is no change in On*Read, we will need to
  // update the receiver when the config becomes complete.
  need_update_ = true;
  // Empty config is considered invalid.
  last_sent_empty_ = true;
  callback_.Run(DnsConfig());
}

void DnsConfigService::OnCompleteConfig() {
  timer_.Stop();
  if (!need_update_)
    return;
  need_update_ = false;
  last_sent_empty_ = false;
  if (watch_failed_) {
    // If a watch failed, the config may not be accurate, so report empty.
    callback_.Run(DnsConfig());
  } else {
    callback_.Run(dns_config_);
  }
}

void DnsConfigService::OnConfigChanged(bool succeeded) {
  if (config_change_delay_) {
    // Ignore transient flutter of config source by delaying the signal a bit.
    cr::SequencedTaskRunnerHandle::Get()->PostDelayedTask(
        CR_FROM_HERE,
        cr::BindOnce(&DnsConfigService::OnConfigChangedDelayed,
                      weak_factory_.GetWeakPtr(), succeeded),
        config_change_delay_.value());
  } else {
    OnConfigChangedDelayed(succeeded);
  }
}

void DnsConfigService::OnHostsChanged(bool succeeded) {
  InvalidateHosts();
  if (succeeded) {
    ReadHostsNow();
  } else {
    CR_LOG(Error) << "DNS hosts watch failed.";
    watch_failed_ = true;
  }
}

void DnsConfigService::OnConfigChangedDelayed(bool succeeded) {
  InvalidateConfig();
  if (succeeded) {
    ReadConfigNow();
  } else {
    CR_LOG(Error) << "DNS config watch failed.";
    watch_failed_ = true;
  }
}

}  // namespace net
}  // namespace cr
