// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_net/network/network_activity_monitor.h"

namespace crnet {

NetworkActivityMonitor::NetworkActivityMonitor() : bytes_received_(0) {}

NetworkActivityMonitor::~NetworkActivityMonitor() = default;

// static
NetworkActivityMonitor* NetworkActivityMonitor::GetInstance() {
  return cr::Singleton<NetworkActivityMonitor>::get();
}

void NetworkActivityMonitor::IncrementBytesReceived(uint64_t bytes_received) {
  cr::AutoLock lock(lock_);
  bytes_received_ += bytes_received;
}

uint64_t NetworkActivityMonitor::GetBytesReceived() const {
  cr::AutoLock lock(lock_);
  return bytes_received_;
}

}  // namespace crnet