// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_ATOMIC_ATOMIC_SEQUENCE_NUM_H_
#define MINI_CHROMIUM_SRC_CRBASE_ATOMIC_ATOMIC_SEQUENCE_NUM_H_

#include <atomic>

#include "crbuild/build_config.h"

namespace cr {

// AtomicSequenceNumber is a thread safe increasing sequence number generator.
// Its constructor doesn't emit a static initializer, so it's safe to use as a
// global variable or static member.
class AtomicSequenceNumber {
 public:
#if defined(MINI_CHROMIUM_ARCH_CPU_64_BITS)
   using IntType = uint64_t;
#else
   using IntType = uint32_t;
#endif

  constexpr AtomicSequenceNumber() = default;
  AtomicSequenceNumber(const AtomicSequenceNumber&) = delete;
  AtomicSequenceNumber& operator=(const AtomicSequenceNumber&) = delete;

  // Returns an increasing sequence number starts from 0 for each call.
  // This function can be called from any thread without data race.
  inline IntType GetNext() {
    return seq_.fetch_add(1, std::memory_order_relaxed); 
  }

 private:
  std::atomic<IntType> seq_{0};
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_ATOMIC_ATOMIC_SEQUENCE_NUM_H_
