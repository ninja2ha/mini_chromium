// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// VERSION: 81

#ifndef MINI_CHROMIUM_SRC_CRNET_NETWORK_WIN_NETWORK_CHANGE_NOTIFIER_WIN_H_
#define MINI_CHROMIUM_SRC_CRNET_NETWORK_WIN_NETWORK_CHANGE_NOTIFIER_WIN_H_

#include <memory>

#include "cr_base/functional/callback.h"
#include "cr_base/memory/ref_ptr.h"
#include "cr_base/memory/weak_ptr.h"
#include "cr_base/threading/sequence_checker.h"
#include "cr_base/win/windows_types.h"
#include "cr_event/win/object_watcher.h"
#include "cr_event/timer/timer.h"
#include "cr_net/net_export.h"
#include "cr_net/network/network_change_notifier.h"
#include "cr_build/compiler_specific.h"

namespace cr {
class SequencedTaskRunner;
}  // namespace cr

namespace crnet {

// NetworkChangeNotifierWin uses a SequenceChecker, as all its internal
// notification code must be called on the sequence it is created and destroyed
// on.  All the NetworkChangeNotifier methods it implements are threadsafe.
class CRNET_EXPORT NetworkChangeNotifierWin
    : public NetworkChangeNotifier,
      public cr::win::ObjectWatcher::Delegate {
 public:
  NetworkChangeNotifierWin(const NetworkChangeNotifierWin&) = delete;
  NetworkChangeNotifierWin& operator=(const NetworkChangeNotifierWin&) = delete;

  NetworkChangeNotifierWin();
  ~NetworkChangeNotifierWin() override;

  // Begins listening for a single subsequent address change.  If it fails to
  // start watching, it retries on a timer.  Must be called only once, on the
  // sequence |this| was created on.  This cannot be called in the constructor,
  // as WatchForAddressChangeInternal is mocked out in unit tests.
  // TODO(mmenke): Consider making this function a part of the
  //               NetworkChangeNotifier interface, so other subclasses can be
  //               unit tested in similar fashion, as needed.
  void WatchForAddressChange();

 protected:
  // For unit tests only.
  bool is_watching() { return is_watching_; }
  void set_is_watching(bool is_watching) { is_watching_ = is_watching; }
  int sequential_failures() { return sequential_failures_; }

 private:
  class DnsConfigServiceThread;
  friend class NetworkChangeNotifierWinTest;
  ///friend class TestNetworkChangeNotifierWin;

  // NetworkChangeNotifier methods:
  ConnectionType GetCurrentConnectionType() const override;

  // ObjectWatcher::Delegate methods:
  // Must only be called on the sequence |this| was created on.
  void OnObjectSignaled(HANDLE object) override;

  // Does the actual work to determine the current connection type.
  // It is not thread safe, see crbug.com/324913.
  static ConnectionType RecomputeCurrentConnectionType();

  // Calls RecomputeCurrentConnectionTypeImpl on the DNS sequence and runs
  // |reply_callback| with the type on the calling sequence.
  virtual void RecomputeCurrentConnectionTypeOnBlockingSequence(
      cr::OnceCallback<void(ConnectionType)> reply_callback) const;

  void SetCurrentConnectionType(ConnectionType connection_type);

  // Notifies IP address change observers of a change immediately, and notifies
  // network state change observers on a delay.  Must only be called on the
  // sequence |this| was created on.
  void NotifyObservers(ConnectionType connection_type);

  // Forwards connection type notifications to parent class.
  void NotifyParentOfConnectionTypeChange();
  void NotifyParentOfConnectionTypeChangeImpl(ConnectionType connection_type);

  // Tries to start listening for a single subsequent address change.  Returns
  // false on failure.  The caller is responsible for updating |is_watching_|.
  // Virtual for unit tests.  Must only be called on the sequence |this| was
  // created on.
  virtual bool WatchForAddressChangeInternal();

  static NetworkChangeCalculatorParams NetworkChangeCalculatorParamsWin();

  // All member variables may only be accessed on the sequence |this| was
  // created on.

  // False when not currently watching for network change events.  This only
  // happens on initialization and when WatchForAddressChangeInternal fails and
  // there is a pending task to try again.  Needed for safe cleanup.
  bool is_watching_;

  cr::win::ObjectWatcher addr_watcher_;
  OVERLAPPED addr_overlapped_;

  cr::OneShotTimer timer_;

  // Number of times WatchForAddressChange has failed in a row.
  int sequential_failures_;

  // Thread on which we can run DnsConfigService.
  std::unique_ptr<DnsConfigServiceThread> dns_config_service_thread_;

  mutable cr::Lock last_computed_connection_type_lock_;
  ConnectionType last_computed_connection_type_;

  // Result of IsOffline() when NotifyObserversOfConnectionTypeChange()
  // was last called.
  bool last_announced_offline_;
  // Number of times polled to check if still offline.
  int offline_polls_;

  CR_SEQUENCE_CHECKER(sequence_checker_);

  // Used for calling WatchForAddressChange again on failure.
  cr::WeakPtrFactory<NetworkChangeNotifierWin> weak_factory_{this};
};

}  // namespace crnet

#endif  // MINI_CHROMIUM_SRC_CRNET_NETWORK_WIN_NETWORK_CHANGE_NOTIFIER_WIN_H_