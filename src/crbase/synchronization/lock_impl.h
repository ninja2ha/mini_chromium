// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 61.0.3163.141

#ifndef MINI_CHROMIUM_SRC_CRBASE_SYNCHRONIZATION_LOCK_IMPL_H_
#define MINI_CHROMIUM_SRC_CRBASE_SYNCHRONIZATION_LOCK_IMPL_H_

#include "crbase/base_export.h"
#include "crbuild/build_config.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "crbase/win/windows_types.h"
#elif defined(MINI_CHROMIUM_OS_POSIX)
#include <errno.h>
#include <pthread.h>
#endif

namespace cr {
namespace internal {

// This class implements the underlying platform-specific spin-lock mechanism
// used for the Lock class.  Most users should not use LockImpl directly, but
// should instead use Lock.
class CRBASE_EXPORT LockImpl {
 public:
#if defined(MINI_CHROMIUM_OS_WIN)
   using NativeHandle = CRITICAL_SECTION;
#elif defined(MINI_CHROMIUM_OS_POSIX)
  using NativeHandle = pthread_mutex_t;
#endif

  LockImpl();
  ~LockImpl();

  // If the lock is not held, take it and return true.  If the lock is already
  // held by something else, immediately return false.
  bool Try();

  // Take the lock, blocking until it is available if necessary.
  void Lock();

  // Release the lock.  This must only be called by the lock's holder: after
  // a successful call to Try, or a call to Lock.
  void Unlock();

  // Return the native underlying lock.
  // TODO(awalker): refactor lock and condition variables so that this is
  // unnecessary.
  NativeHandle* native_handle() { return &native_handle_; }

#if defined(MINI_CHROMIUM_OS_POSIX)
  // Whether this lock will attempt to use priority inheritance.
  static bool PriorityInheritanceAvailable();
#endif

 private:
  NativeHandle native_handle_;
};


// This is an implementation used for AutoLock templated on the lock type.
template <class LockType>
class BasicAutoLock {
 public:
  struct AlreadyAcquired {};

  BasicAutoLock(const BasicAutoLock&) = delete;
  BasicAutoLock& operator=(const BasicAutoLock&) = delete;

  explicit BasicAutoLock(LockType& lock)
      : lock_(lock) {
    lock_.Acquire();
  }

  BasicAutoLock(LockType& lock, const AlreadyAcquired&)
      : lock_(lock) {
    lock_.AssertAcquired();
  }

  ~BasicAutoLock() {
    lock_.AssertAcquired();
    lock_.Release();
  }

 private:
  LockType& lock_;
};

// This is an implementation used for AutoUnlock templated on the lock type.
template <class LockType>
class BasicAutoUnlock {
 public:
  BasicAutoUnlock(const BasicAutoUnlock&) = delete;
  BasicAutoUnlock& operator=(const BasicAutoUnlock&) = delete;

  explicit BasicAutoUnlock(LockType& lock) : lock_(lock) {
    // We require our caller to have the lock.
    lock_.AssertAcquired();
    lock_.Release();
  }

  ~BasicAutoUnlock() { lock_.Acquire(); }

 private:
  LockType& lock_;
};

// This is an implementation used for AutoLockMaybe templated on the lock type.
template <class LockType>
class BasicAutoLockMaybe {
 public:
  BasicAutoLockMaybe(const BasicAutoLockMaybe&) = delete;
  BasicAutoLockMaybe& operator=(const BasicAutoLockMaybe&) = delete;

  explicit BasicAutoLockMaybe(LockType* lock)
      : lock_(lock) {
    if (lock_)
      lock_->Acquire();
  }

  ~BasicAutoLockMaybe() {
    if (lock_) {
      lock_->AssertAcquired();
      lock_->Release();
    }
  }

 private:
  LockType* const lock_;
};

// This is an implementation used for ReleasableAutoLock templated on the lock
// type.
template <class LockType>
class BasicReleasableAutoLock {
 public:
  BasicReleasableAutoLock(const BasicReleasableAutoLock&) = delete;
  BasicReleasableAutoLock& operator=(const BasicReleasableAutoLock&) = delete;

  explicit BasicReleasableAutoLock(LockType* lock)
      : lock_(lock) {
    CR_DCHECK(lock_);
    lock_->Acquire();
  }

  ~BasicReleasableAutoLock() {
    if (lock_) {
      lock_->AssertAcquired();
      lock_->Release();
    }
  }

  void Release() {
    CR_DCHECK(lock_);
    lock_->AssertAcquired();
    lock_->Release();
    lock_ = nullptr;
  }

 private:
  LockType* lock_;
};

}  // namespace internal
}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_SYNCHRONIZATION_LOCK_IMPL_H_