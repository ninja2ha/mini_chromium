// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#ifndef MINI_CHROMIUM_SRC_CRBASE_RT_TASK_COMMON_CHECKED_LOCK_IMPL_H_
#define MINI_CHROMIUM_SRC_CRBASE_RT_TASK_COMMON_CHECKED_LOCK_IMPL_H_

#include <memory>

#include "crbase/base_export.h"
#include "crbase/synchronization/lock.h"

namespace cr {

class ConditionVariable;

namespace internal {

struct UniversalPredecessor {};
struct UniversalSuccessor {};

// A regular lock with simple deadlock correctness checking.
// This lock tracks all of the available locks to make sure that any locks are
// acquired in an expected order.
// See scheduler_lock.h for details.
class CRBASE_EXPORT CheckedLockImpl {
 public:
  CheckedLockImpl(const CheckedLockImpl&) = delete;
  CheckedLockImpl& operator=(const CheckedLockImpl&) = delete;

  CheckedLockImpl();
  explicit CheckedLockImpl(const CheckedLockImpl* predecessor);
  explicit CheckedLockImpl(UniversalPredecessor);
  explicit CheckedLockImpl(UniversalSuccessor);
  ~CheckedLockImpl();

  static void AssertNoLockHeldOnCurrentThread();

  void Acquire();
  void Release();

  void AssertAcquired() const;

  std::unique_ptr<ConditionVariable> CreateConditionVariable();

  bool is_universal_predecessor() const { return is_universal_predecessor_; }
  bool is_universal_successor() const { return is_universal_successor_; }

 private:
  Lock lock_;
  const bool is_universal_predecessor_ = false;
  const bool is_universal_successor_ = false;
};

}  // namespace internal
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_RT_TASK_COMMON_CHECKED_LOCK_IMPL_H_
