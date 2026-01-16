// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

// WARNING: You should *NOT* be using this class directly.  PlatformThread is
// the low-level platform-specific abstraction to the OS's threading interface.
// You should instead be using a message-loop driven Thread, see thread.h.

#ifndef MINI_CHROMIUM_SRC_CRBASE_THREADING_PLATFORM_THREAD_H_
#define MINI_CHROMIUM_SRC_CRBASE_THREADING_PLATFORM_THREAD_H_

#include <stddef.h>

#include "crbase/base_export.h"
#include "crbase/time/time.h"
#include "cr/build_config.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "crbase/win/windows_types.h"
#elif defined(MINI_CHROMIUM_OS_POSIX)
#include <pthread.h>
#include <unistd.h>
#endif

namespace cr {

// Used for logging. Always an integer value.
#if defined(MINI_CHROMIUM_OS_WIN)
typedef DWORD PlatformThreadId;
#elif defined(MINI_CHROMIUM_OS_POSIX)
typedef pid_t PlatformThreadId;
#endif

// Used for thread checking and debugging.
// Meant to be as fast as possible.
// These are produced by PlatformThread::CurrentRef(), and used to later
// check if we are on the same thread or not by using ==. These are safe
// to copy between threads, but can't be copied to another process as they
// have no meaning there. Also, the internal identifier can be re-used
// after a thread dies, so a PlatformThreadRef cannot be reliably used
// to distinguish a new thread from an old, dead thread.
class PlatformThreadRef {
 public:
#if defined(MINI_CHROMIUM_OS_WIN)
  typedef DWORD RefType;
#else  //  MINI_CHROMIUM_OS_POSIX
  typedef pthread_t RefType;
#endif
  constexpr PlatformThreadRef() : id_(0) {}

  explicit constexpr PlatformThreadRef(RefType id) : id_(id) {}

  bool operator==(PlatformThreadRef other) const {
    return id_ == other.id_;
  }

  bool operator!=(PlatformThreadRef other) const { return id_ != other.id_; }

  bool is_null() const {
    return id_ == 0;
  }
 private:
  RefType id_;
};

// Used to operate on threads.
class PlatformThreadHandle {
 public:
#if defined(MINI_CHROMIUM_OS_WIN)
  typedef void* Handle;
#elif defined(MINI_CHROMIUM_OS_POSIX)
  typedef pthread_t Handle;
#endif

  constexpr PlatformThreadHandle() : handle_(0) {}

  explicit constexpr PlatformThreadHandle(Handle handle) : handle_(handle) {}

  bool is_equal(const PlatformThreadHandle& other) const {
    return handle_ == other.handle_;
  }

  bool is_null() const {
    return !handle_;
  }

  Handle platform_handle() const {
    return handle_;
  }

 private:
  Handle handle_;
};

const PlatformThreadId kInvalidThreadId(0);

// A namespace for low-level thread functions.
class CRBASE_EXPORT PlatformThread {
 public:
  PlatformThread() = delete;
  PlatformThread(const PlatformThread&) = delete;
  PlatformThread& operator=(const PlatformThread&) = delete;

  // Gets the current thread id, which may be useful for logging purposes.
  static PlatformThreadId CurrentId();

  // Gets the current thread reference, which can be used to check if
  // we're on the right thread quickly.
  static PlatformThreadRef CurrentRef();

  // Get the handle representing the current thread. On Windows, this is a
  // pseudo handle constant which will always represent the thread using it and
  // hence should not be shared with other threads nor be used to differentiate
  // the current thread from another.
  static PlatformThreadHandle CurrentHandle();

  // Yield the current thread so another thread can be scheduled.
  static void YieldCurrentThread();

  // Sleeps for the specified duration (real-time; ignores time overrides).
  // Note: The sleep duration may be in cr::Time or cr::TimeTicks, depending
  // on platform. If you're looking to use this in unit tests testing delayed
  // tasks, this will be unreliable - instead, use
  // cr::test::TaskEnvironment with MOCK_TIME mode.
  static void Sleep(cr::TimeDelta duration);
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_THREADING_PLATFORM_THREAD_H_
