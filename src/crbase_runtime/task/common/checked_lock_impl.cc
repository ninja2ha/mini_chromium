// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "crbase_runtime/task/common/checked_lock_impl.h"

#include <unordered_map>
#include <vector>

#include "crbase/logging/logging.h"
#include "crbase/memory/singleton.h"
#include "crbase/synchronization/condition_variable.h"
#include "crbase/threading/platform_thread.h"
#include "crbase/threading/thread_local.h"
#include "crbase_runtime/task/common/checked_lock.h"

namespace cr {
namespace internal {

namespace {

class SafeAcquisitionTracker {
 public:
  SafeAcquisitionTracker(const SafeAcquisitionTracker&) = delete;
  SafeAcquisitionTracker& operator=(const SafeAcquisitionTracker&) = delete;

  static SafeAcquisitionTracker* GetInstance();

  SafeAcquisitionTracker() = default;

  void RegisterLock(const CheckedLockImpl* const lock,
                    const CheckedLockImpl* const predecessor) {
    CR_DCHECK(lock != predecessor) << "Reentrant locks are unsupported.";
    AutoLock auto_lock(allowed_predecessor_map_lock_);
    allowed_predecessor_map_[lock] = predecessor;
    AssertSafePredecessor(lock);
  }

  void UnregisterLock(const CheckedLockImpl* const lock) {
    AutoLock auto_lock(allowed_predecessor_map_lock_);
    allowed_predecessor_map_.erase(lock);
  }

  void RecordAcquisition(const CheckedLockImpl* const lock) {
    AssertSafeAcquire(lock);
    GetAcquiredLocksOnCurrentThread()->push_back(lock);
  }

  void RecordRelease(const CheckedLockImpl* const lock) {
    LockVector* acquired_locks = GetAcquiredLocksOnCurrentThread();
    const auto iter_at_lock = 
        std::find(acquired_locks->begin(), acquired_locks->end(), lock);
    CR_DCHECK(iter_at_lock != acquired_locks->end());
    acquired_locks->erase(iter_at_lock);
  }

  void AssertNoLockHeldOnCurrentThread() {
    CR_DCHECK(GetAcquiredLocksOnCurrentThread()->empty());
  }

 private:
  friend struct cr::DefaultSingletonTraits<SafeAcquisitionTracker>;

  using LockVector = std::vector<const CheckedLockImpl*>;
  using PredecessorMap =
      std::unordered_map<const CheckedLockImpl*, const CheckedLockImpl*>;

  // This asserts that the lock is safe to acquire. This means that this should
  // be run before actually recording the acquisition.
  void AssertSafeAcquire(const CheckedLockImpl* const lock) {
    const LockVector* acquired_locks = GetAcquiredLocksOnCurrentThread();

    // If the thread currently holds no locks, this is inherently safe.
    if (acquired_locks->empty())
      return;

    // A universal predecessor may not be acquired after any other lock.
    CR_DCHECK(!lock->is_universal_predecessor());

    // Otherwise, make sure that the previous lock acquired is either an
    // allowed predecessor for this lock or a universal predecessor.
    const CheckedLockImpl* previous_lock = acquired_locks->back();
    if (previous_lock->is_universal_predecessor())
      return;

    AutoLock auto_lock(allowed_predecessor_map_lock_);
    // Using at() is exception-safe here as |lock| was registered already.
    const CheckedLockImpl* allowed_predecessor =
        allowed_predecessor_map_.at(lock);
    if (lock->is_universal_successor()) {
      CR_DCHECK(!previous_lock->is_universal_successor());
      return;
    } else {
      CR_DCHECK(previous_lock == allowed_predecessor);
    }
  }

  // Asserts that |lock|'s registered predecessor is safe. Because
  // CheckedLocks are registered at construction time and any predecessor
  // specified on a CheckedLock must already exist, the first registered
  // CheckedLock in a potential chain must have a null predecessor and is thus
  // cycle-free. Any subsequent CheckedLock with a predecessor must come from
  // the set of registered CheckedLocks. Since the registered CheckedLocks
  // only contain cycle-free CheckedLocks, this subsequent CheckedLock is
  // itself cycle-free and may be safely added to the registered CheckedLock
  // set.
  void AssertSafePredecessor(const CheckedLockImpl* lock) const {
    allowed_predecessor_map_lock_.AssertAcquired();
    // Using at() is exception-safe here as |lock| was registered already.
    const CheckedLockImpl* predecessor = allowed_predecessor_map_.at(lock);
    if (predecessor) {
      CR_DCHECK(allowed_predecessor_map_.find(predecessor) !=
                allowed_predecessor_map_.end())
          << "CheckedLock was registered before its predecessor. "
          << "Potential cycle detected";
    }
  }

  LockVector* GetAcquiredLocksOnCurrentThread() {
    if (!tls_acquired_locks_.Get())
      tls_acquired_locks_.Set(std::make_unique<LockVector>());

    return tls_acquired_locks_.Get();
  }

  // Synchronizes access to |allowed_predecessor_map_|.
  Lock allowed_predecessor_map_lock_;

  // A map of allowed predecessors.
  PredecessorMap allowed_predecessor_map_;

  // A thread-local slot holding a vector of locks currently acquired on the
  // current thread.
  ThreadLocalOwnedPointer<LockVector> tls_acquired_locks_;
};

SafeAcquisitionTracker* SafeAcquisitionTracker::GetInstance() {
  return cr::Singleton<SafeAcquisitionTracker>::get();
}

}  // namespace

CheckedLockImpl::CheckedLockImpl() : CheckedLockImpl(nullptr) {}

CheckedLockImpl::CheckedLockImpl(const CheckedLockImpl* predecessor)
    : is_universal_predecessor_(false) {
  CR_DCHECK(predecessor == nullptr || !predecessor->is_universal_successor_);
  SafeAcquisitionTracker::GetInstance()->RegisterLock(this, predecessor);
}

CheckedLockImpl::CheckedLockImpl(UniversalPredecessor)
    : is_universal_predecessor_(true) {}

CheckedLockImpl::CheckedLockImpl(UniversalSuccessor)
    : is_universal_successor_(true) {
  SafeAcquisitionTracker::GetInstance()->RegisterLock(this, nullptr);
}

CheckedLockImpl::~CheckedLockImpl() {
  SafeAcquisitionTracker::GetInstance()->UnregisterLock(this);
}

void CheckedLockImpl::AssertNoLockHeldOnCurrentThread() {
  SafeAcquisitionTracker::GetInstance()->AssertNoLockHeldOnCurrentThread();
}

void CheckedLockImpl::Acquire() {
  lock_.Acquire();
  SafeAcquisitionTracker::GetInstance()->RecordAcquisition(this);
}

void CheckedLockImpl::Release() {
  lock_.Release();
  SafeAcquisitionTracker::GetInstance()->RecordRelease(this);
}

void CheckedLockImpl::AssertAcquired() const {
  lock_.AssertAcquired();
}

std::unique_ptr<ConditionVariable> CheckedLockImpl::CreateConditionVariable() {
  return std::make_unique<ConditionVariable>(&lock_);
}

}  // namespace internal
}  // namespace cr
