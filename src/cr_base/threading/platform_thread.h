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

#include "cr_base/base_export.h"
#include "cr_base/time/time.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "cr_base/win/window_types.h"
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

const PlatformThreadId kInvalidThreadId(0);

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

  static PlatformThreadHandle Current();

 private:
  Handle handle_;
};


///// Valid values for priority of Thread::Options and SimpleThread::Options, and
///// SetCurrentThreadPriority(), listed in increasing order of importance.
///enum class ThreadPriority : int {
///  // Suitable for threads that shouldn't disrupt high priority work.
///  BACKGROUND,
///  // Default priority level.
///  NORMAL,
///  // Suitable for threads which generate data for the display (at ~60Hz).
///  DISPLAY,
///  // Suitable for low-latency, glitch-resistant audio.
///  REALTIME_AUDIO,
///};

// A namespace for low-level thread functions.
class CRBASE_EXPORT PlatformThread {
 public:
  PlatformThread() = delete;
  PlatformThread(const PlatformThread&) = delete;
  PlatformThread& operator=(const PlatformThread&) = delete;

  // Gets the current thread id, which may be useful for logging purposes.
  static PlatformThreadId CurrentId();

  // Yield the current thread so another thread can be scheduled.
  static void YieldCurrentThread();

  // Sleeps for the specified duration (real-time; ignores time overrides).
  // Note: The sleep duration may be in cr::Time or cr::TimeTicks, depending
  // on platform. 
  static void Sleep(cr::TimeDelta duration);
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_THREADING_PLATFORM_THREAD_H_
