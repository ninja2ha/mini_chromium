// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_net/dns/system_dns_config_change_notifier.h"

#include <map>
#include <utility>

#include "cr_base/logging/logging.h"
#include "cr_base/functional/bind.h"
#include "cr_base/location.h"
#include "cr_base/memory/weak_ptr.h"
#include "cr_base/synchronization/lock.h"
#include "cr_base/threading/sequence/sequence_checker.h"
#include "cr_event/task/sequenced_task_runner.h"
#include "cr_event/task/sequenced_task_runner_handle.h"

#include "cr_net/dns/dns_config_service.h"

namespace cr {
namespace net {

namespace {

// Internal information and handling for a registered Observer. Handles
// posting to and DCHECKing the correct sequence for the Observer.
class WrappedObserver {
 public:
  WrappedObserver(const WrappedObserver&) = delete;
  WrappedObserver& operator=(const WrappedObserver&) = delete;

  explicit WrappedObserver(SystemDnsConfigChangeNotifier::Observer* observer)
      : task_runner_(cr::SequencedTaskRunnerHandle::Get()),
        observer_(observer) {}

  ~WrappedObserver() { CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_); }

  void OnNotifyThreadsafe(cr::Optional<DnsConfig> config) {
    task_runner_->PostTask(
        CR_FROM_HERE,
        cr::BindOnce(&WrappedObserver::OnNotify,
                     weak_ptr_factory_.GetWeakPtr(), std::move(config)));
  }

  void OnNotify(cr::Optional<DnsConfig> config) {
    CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    CR_DCHECK(!config || config.value().IsValid());

    observer_->OnSystemDnsConfigChanged(std::move(config));
  }

 private:
  RefPtr<cr::SequencedTaskRunner> task_runner_;
  SystemDnsConfigChangeNotifier::Observer* const observer_;

  CR_SEQUENCE_CHECKER(sequence_checker_);
  cr::WeakPtrFactory<WrappedObserver> weak_ptr_factory_{this};
};

}  // namespace

// Internal core to be destroyed via base::OnTaskRunnerDeleter to ensure
// sequence safety.
class SystemDnsConfigChangeNotifier::Core {
 public:
  Core(const Core&) = delete;
  Core& operator=(const Core&) = delete;

  Core(RefPtr<cr::SequencedTaskRunner> task_runner,
       std::unique_ptr<DnsConfigService> dns_config_service)
      : task_runner_(std::move(task_runner)) {
    CR_DCHECK(task_runner_);
    CR_DCHECK(dns_config_service);

    CR_DETACH_FROM_SEQUENCE(sequence_checker_);

    task_runner_->PostTask(CR_FROM_HERE,
                           cr::BindOnce(&Core::SetAndStartDnsConfigService,
                                        weak_ptr_factory_.GetWeakPtr(),
                                        std::move(dns_config_service)));
  }

  ~Core() {
    CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    CR_DCHECK(wrapped_observers_.empty());
  }

  void AddObserver(Observer* observer) {
    // Create wrapped observer outside locking in case construction requires
    // complex side effects.
    auto wrapped_observer = std::make_unique<WrappedObserver>(observer);

    {
      cr::AutoLock lock(lock_);

      if (config_) {
        // Even though this is the same sequence as the observer, use the
        // threadsafe OnNotify to post the notification for both lock and
        // reentrancy safety.
        wrapped_observer->OnNotifyThreadsafe(config_);
      }

      CR_DCHECK(0u == wrapped_observers_.count(observer));
      wrapped_observers_.emplace(observer, std::move(wrapped_observer));
    }
  }

  void RemoveObserver(Observer* observer) {
    // Destroy wrapped observer outside locking in case destruction requires
    // complex side effects.
    std::unique_ptr<WrappedObserver> removed_wrapped_observer;

    {
      cr::AutoLock lock(lock_);
      auto it = wrapped_observers_.find(observer);
      CR_DCHECK(it != wrapped_observers_.end());
      removed_wrapped_observer = std::move(it->second);
      wrapped_observers_.erase(it);
    }
  }

  void RefreshConfig() {
    task_runner_->PostTask(CR_FROM_HERE,
                           cr::BindOnce(&Core::TriggerRefreshConfig,
                                        weak_ptr_factory_.GetWeakPtr()));
  }

  void SetDnsConfigServiceForTesting(
      std::unique_ptr<DnsConfigService> dns_config_service) {
    CR_DCHECK(dns_config_service);
    task_runner_->PostTask(CR_FROM_HERE,
                           cr::BindOnce(&Core::SetAndStartDnsConfigService,
                                        weak_ptr_factory_.GetWeakPtr(),
                                        std::move(dns_config_service)));
  }

 private:
  void SetAndStartDnsConfigService(
      std::unique_ptr<DnsConfigService> dns_config_service) {
    CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

    dns_config_service_ = std::move(dns_config_service);
    dns_config_service_->WatchConfig(cr::BindRepeating(
        &Core::OnConfigChanged, weak_ptr_factory_.GetWeakPtr()));
  }

  void OnConfigChanged(const DnsConfig& config) {
    CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    cr::AutoLock lock(lock_);

    // |config_| is |cr::nullopt| if most recent config was invalid (or no
    // valid config has yet been read), so convert |config| to a similar form
    // before comparing for change.
    cr::Optional<DnsConfig> new_config;
    if (config.IsValid())
      new_config = config;

    if (config_ == new_config)
      return;

    config_ = std::move(new_config);

    for (auto& wrapped_observer : wrapped_observers_) {
      wrapped_observer.second->OnNotifyThreadsafe(config_);
    }
  }

  void TriggerRefreshConfig() {
    CR_DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    dns_config_service_->RefreshConfig();
  }

  // Fields that may be accessed from any sequence. Must protect access using
  // |lock_|.
  mutable cr::Lock lock_;
  // Only stores valid configs. |base::nullopt| if most recent config was
  // invalid (or no valid config has yet been read).
  cr::Optional<DnsConfig> config_;
  std::map<Observer*, std::unique_ptr<WrappedObserver>> wrapped_observers_;

  // Fields valid only on |task_runner_|.
  RefPtr<cr::SequencedTaskRunner> task_runner_;
  CR_SEQUENCE_CHECKER(sequence_checker_);
  std::unique_ptr<DnsConfigService> dns_config_service_;
  cr::WeakPtrFactory<Core> weak_ptr_factory_{this};
};

///SystemDnsConfigChangeNotifier::SystemDnsConfigChangeNotifier()
///    : SystemDnsConfigChangeNotifier(
///          cr::ThreadPool::CreateSequencedTaskRunner({base::MayBlock()}),
///          DnsConfigService::CreateSystemService()) {}

SystemDnsConfigChangeNotifier::SystemDnsConfigChangeNotifier(
    RefPtr<cr::SequencedTaskRunner> task_runner,
    std::unique_ptr<DnsConfigService> dns_config_service)
    : core_(nullptr, cr::OnTaskRunnerDeleter(task_runner)) {
  if (dns_config_service)
    core_.reset(new Core(task_runner, std::move(dns_config_service)));
}

SystemDnsConfigChangeNotifier::~SystemDnsConfigChangeNotifier() = default;

void SystemDnsConfigChangeNotifier::AddObserver(Observer* observer) {
  if (core_)
    core_->AddObserver(observer);
}

void SystemDnsConfigChangeNotifier::RemoveObserver(Observer* observer) {
  if (core_)
    core_->RemoveObserver(observer);
}

void SystemDnsConfigChangeNotifier::RefreshConfig() {
  if (core_)
    core_->RefreshConfig();
}

void SystemDnsConfigChangeNotifier::SetDnsConfigServiceForTesting(
    std::unique_ptr<DnsConfigService> dns_config_service) {
  CR_DCHECK(core_);
  CR_DCHECK(dns_config_service);

  core_->SetDnsConfigServiceForTesting(std::move(dns_config_service));
}

}  // namespace net
}  // namespace cr
