// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_SYNCHRONIZATION_ATOMIC_FLAG_H_
#define MINI_CHROMIUM_SRC_CRBASE_SYNCHRONIZATION_ATOMIC_FLAG_H_

#include <stdint.h>

#include <atomic>

#include "crbase/base_export.h"
#include "crbase/threading/sequence_checker.h"

namespace cr {

// A flag that can safely be set from one thread and read from other threads.
//
// This class IS NOT intended for synchronization between threads.
class CRBASE_EXPORT AtomicFlag {
 public:
  AtomicFlag(const AtomicFlag&) = delete;
  AtomicFlag& operator=(const AtomicFlag&) = delete;

  AtomicFlag();
  ~AtomicFlag();

  // Set the flag. Must always be called from the same sequence.
  void Set();

  // Returns true iff the flag was set. If this returns true, the current thread
  // is guaranteed to be synchronized with all memory operations on the sequence
  // which invoked Set() up until at least the first call to Set() on it.
  bool IsSet() const {
    // Inline here: this has a measurable performance impact on cr::WeakPtr.
    return flag_.load(std::memory_order_acquire) != 0;
  }

  // Resets the flag. Be careful when using this: callers might not expect
  // IsSet() to return false after returning true once.
  void UnsafeResetForTesting();

 private:
  std::atomic<uint_fast8_t> flag_{0};
  CR_SEQUENCE_CHECKER(set_sequence_checker_);
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_SYNCHRONIZATION_ATOMIC_FLAG_H_
