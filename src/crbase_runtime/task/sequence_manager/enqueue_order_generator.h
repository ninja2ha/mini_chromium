
// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_RT_TASK_SEQUENCE_MANAGER_ENQUEUE_ORDER_GENERATOR_H_
#define MINI_CHROMIUM_SRC_CRBASE_RT_TASK_SEQUENCE_MANAGER_ENQUEUE_ORDER_GENERATOR_H_

#include <stdint.h>

#include <atomic>

#include "crbase_runtime/runtime_export.h"
#include "crbase_runtime/task/sequence_manager/enqueue_order.h"

namespace cr {
namespace sequence_manager {
namespace internal {

// EnqueueOrder can't be created from a raw number in non-test code.
// EnqueueOrderGenerator is used to create it with strictly monotonic guarantee.
class CRBASE_RT_EXPORT EnqueueOrderGenerator {
 public:
  EnqueueOrderGenerator();
  EnqueueOrderGenerator(const EnqueueOrderGenerator&) = delete;
  EnqueueOrderGenerator& operator=(const EnqueueOrderGenerator&) = delete;
  ~EnqueueOrderGenerator();

  // Can be called from any thread.
  EnqueueOrder GenerateNext() {
    return EnqueueOrder(std::atomic_fetch_add_explicit(
        &counter_, uint64_t(1), std::memory_order_relaxed));
  }

 private:
  std::atomic<uint64_t> counter_;
};

}  // namespace internal
}  // namespace sequence_manager
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_RT_TASK_SEQUENCE_MANAGER_ENQUEUE_ORDER_GENERATOR_H_
