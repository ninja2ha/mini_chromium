// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRNET_DNS_DNS_CONFIG_SERVICE_H_
#define MINI_CHROMIUM_SRC_CRNET_DNS_DNS_CONFIG_SERVICE_H_

#include <map>
#include <memory>

#include "cr_base/files/file_path.h"
#include "cr_base/memory/ref_ptr.h"
#include "cr_base/memory/weak_ptr.h"
#include "cr_base/containers/optional.h"
#include "cr_base/threading/sequence/sequence_checker.h"
#include "cr_base/time/time.h"

#include "cr_event/timer/timer.h"

#include "cr_net/base/net_export.h"
#include "cr_net/dns/dns_config.h"
#include "cr_net/dns/dns_hosts.h"
#include "cr_net/dns/serial_worker.h"
///#include "url/gurl.h"

namespace cr {
namespace net {

// Service for reading system DNS settings, on demand or when signalled by
// internal watchers and NetworkChangeNotifier. This object is not thread-safe
// and methods may perform blocking I/O so methods must be called on a sequence
// that allows blocking (i.e. base::MayBlock).
class CRNET_EXPORT DnsConfigService {
 public:
  // Callback interface for the client, called on the same thread as
  // ReadConfig() and WatchConfig().
  typedef cr::RepeatingCallback<void(const DnsConfig& config)> CallbackType;

  // DHCP and user-induced changes are on the order of seconds, so 150ms should
  // not add perceivable delay. On the other hand, config readers should finish
  // within 150ms with the rare exception of I/O block or extra large HOSTS.
  static const cr::TimeDelta kInvalidationTimeout;

  // Creates the platform-specific DnsConfigService. May return |nullptr| if
  // reading system DNS settings is not supported on the current platform.
  static std::unique_ptr<DnsConfigService> CreateSystemService();

  // On detecting config change, will post and wait `config_change_delay` before
  // triggering refreshes. Will trigger refreshes synchronously on nullopt.
  // Useful for platforms where multiple changes may be made and detected before
  // the config is stabilized and ready to be read.
  explicit DnsConfigService(
      cr::FilePath::StringPieceType hosts_file_path,
      cr::Optional<cr::TimeDelta> config_change_delay =
          cr::TimeDelta::FromMilliseconds(50));
  virtual ~DnsConfigService();

  // Attempts to read the configuration. Will run |callback| when succeeded.
  // Can be called at most once.
  void ReadConfig(const CallbackType& callback);

  // Registers systems watchers. Will attempt to read config after watch starts,
  // but only if watchers started successfully. Will run |callback| iff config
  // changes from last call or has to be withdrawn. Can be called at most once.
  // Might require MessageLoopForIO.
  void WatchConfig(const CallbackType& callback);

  // Triggers invalidation and re-read of the current configuration (followed by
  // invocation of the callback). For use only on platforms expecting
  // network-stack-external notifications of DNS config changes.
  virtual void RefreshConfig();

  void set_watch_failed_for_testing(bool watch_failed) {
    watch_failed_ = watch_failed;
  }

 protected:
  // Watcher to observe for changes to DNS config or HOSTS (via overriding
  // `Watch()` with platform specifics) and trigger necessary refreshes on
  // changes.
  class Watcher {
   public:
    // `service` is expected to own the created Watcher and thus stay valid for
    // the lifetime of the created Watcher.
    explicit Watcher(DnsConfigService& service);
    virtual ~Watcher();

    Watcher(const Watcher&) = delete;
    Watcher& operator=(const Watcher&) = delete;

    virtual bool Watch() = 0;

   protected:
    // Hooks for detected changes. `succeeded` false to indicate that there was
    // an error watching for the change.
    void OnConfigChanged(bool succeeded);
    void OnHostsChanged(bool succeeded);

    void CheckOnCorrectSequence();

   private:
    void OnConfigChangedDelayed(bool success);

    // Back pointer. `this` is expected to be owned by `service_`, making this
    // raw pointer safe.
    DnsConfigService* const service_;

    CR_SEQUENCE_CHECKER(sequence_checker_);
  };

  // Reader of HOSTS files. In this base implementation, uses standard logic
  // appropriate to most platforms to read the HOSTS file located at
  // `hosts_file_path`.
  class HostsReader : public SerialWorker {
   public:
    HostsReader(const HostsReader&) = delete;
    HostsReader& operator=(const HostsReader&) = delete;

    // `service` is expected to own the created reader and thus stay valid for
    // the lifetime of the created reader.
    HostsReader(cr::FilePath::StringPieceType hosts_file_path,
                DnsConfigService& service);
   protected:
    ~HostsReader() override;

    // Reads the HOSTS file and parses to a `DnsHosts`. Returns nullopt on
    // failure. Will be called on a separate blockable ThreadPool thread.
    //
    // Override if needed to implement platform-specific behavior, e.g. for a
    // platform-specific HOSTS format.
    virtual cr::Optional<DnsHosts> ReadHosts();

    // Adds any necessary additional entries to the given `DnsHosts`. Returns
    // false on failure. Will be called on a separate blockable ThreadPool
    // thread.
    //
    // Override if needed to implement platform-specific behavior.
    virtual bool AddAdditionalHostsTo(DnsHosts& in_out_dns_hosts);

    // SerialWorker:
    void DoWork() final;
    void OnWorkFinished() final;

   private:
    // Raw pointer to owning DnsConfigService. This must never be accessed
    // inside DoWork(), since service may be destroyed while SerialWorker is
    // running on worker thread.
    DnsConfigService* const service_;
    // Written in DoWork, read in OnWorkFinished, no locking necessary.
    cr::Optional<DnsHosts> hosts_;

    const cr::FilePath hosts_file_path_;
  };

  // Immediately attempts to read the current configuration.
  virtual void ReadConfigNow() = 0;
  virtual void ReadHostsNow();
  // Registers system watchers. Returns true iff succeeds.
  virtual bool StartWatching() = 0;

  // Called when the current config (except hosts) has changed.
  void InvalidateConfig();
  // Called when the current hosts have changed.
  void InvalidateHosts();

  // Called with new config. |config|.hosts is ignored.
  void OnConfigRead(DnsConfig config);
  // Called with new hosts. Rest of the config is assumed unchanged.
  void OnHostsRead(DnsHosts hosts);

  CR_SEQUENCE_CHECKER(sequence_checker_);

 private:
  // The timer counts from the last Invalidate* until complete config is read.
  void StartTimer();
  void OnTimeout();
  // Called when the config becomes complete. Stops the timer.
  void OnCompleteConfig();

  // Hooks for Watcher change notifications. `succeeded` false to indicate that
  // there was an error watching for the change.
  void OnConfigChanged(bool succeeded);
  void OnHostsChanged(bool succeeded);
  void OnConfigChangedDelayed(bool succeeded);

  CallbackType callback_;

  DnsConfig dns_config_;

  // True if any of the necessary watchers failed. In that case, the service
  // will communicate changes via OnTimeout, but will only send empty DnsConfig.
  bool watch_failed_;
  // True after On*Read, before Invalidate*. Tells if the config is complete.
  bool have_config_;
  bool have_hosts_;
  // True if receiver needs to be updated when the config becomes complete.
  bool need_update_;
  // True if the last config sent was empty (instead of |dns_config_|).
  // Set when |timer_| expires.
  bool last_sent_empty_;

  const cr::Optional<cr::TimeDelta> config_change_delay_;
  const cr::FilePath hosts_file_path_;

  // Created only if needed in ReadHostsNow() to avoid creating unnecessarily if
  // overridden for a platform-specific implementation.
  RefPtr<HostsReader> hosts_reader_;

  // Started in Invalidate*, cleared in On*Read.
  cr::OneShotTimer timer_;

  cr::WeakPtrFactory<DnsConfigService> weak_factory_{this};
};

}  // namespace net
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRNET_DNS_DNS_CONFIG_SERVICE_H_
