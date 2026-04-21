// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "cr_net/socket/udp/udp_socket_global_limits.h"

#include <limits>

#include "cr_base/atomic/atomic_ref_count.h"
#include "cr_base/memory/no_destructor.h"

namespace crnet {

namespace {

// Threadsafe singleton for tracking the process-wide count of UDP sockets.
class GlobalUDPSocketCounts {
 public:
  GlobalUDPSocketCounts() : count_(0) {}

  ~GlobalUDPSocketCounts() = delete;

  static GlobalUDPSocketCounts& Get() {
    static cr::NoDestructor<GlobalUDPSocketCounts> singleton;
    return *singleton;
  }

  bool TryAcquireSocket() CR_WARN_UNUSED_RESULT {
    auto previous = count_.Increment(1);
    if (previous >= GetMax()) {
      count_.Decrement();
      return false;
    }

    return true;
  }

  cr::AtomicRefCount::IntType GetMax() {
    ///if (base::FeatureList::IsEnabled(features::kLimitOpenUDPSockets))
    ///  return features::kLimitOpenUDPSocketsMax.Get();

    return std::numeric_limits<cr::AtomicRefCount::IntType>::max();
  }

  void ReleaseSocket() { count_.Increment(-1); }

 private:
  cr::AtomicRefCount count_;
};

}  // namespace

OwnedUDPSocketCount::OwnedUDPSocketCount() : OwnedUDPSocketCount(true) {}

OwnedUDPSocketCount::OwnedUDPSocketCount(OwnedUDPSocketCount&& other) {
  *this = std::move(other);
}

OwnedUDPSocketCount& OwnedUDPSocketCount::operator=(
    OwnedUDPSocketCount&& other) {
  Reset();
  empty_ = other.empty_;
  other.empty_ = true;
  return *this;
}

OwnedUDPSocketCount::~OwnedUDPSocketCount() {
  Reset();
}

void OwnedUDPSocketCount::Reset() {
  if (!empty_) {
    GlobalUDPSocketCounts::Get().ReleaseSocket();
    empty_ = true;
  }
}

OwnedUDPSocketCount::OwnedUDPSocketCount(bool empty) : empty_(empty) {}

OwnedUDPSocketCount TryAcquireGlobalUDPSocketCount() {
  bool success = GlobalUDPSocketCounts::Get().TryAcquireSocket();
  return OwnedUDPSocketCount(!success);
}

}  // namespace crnet