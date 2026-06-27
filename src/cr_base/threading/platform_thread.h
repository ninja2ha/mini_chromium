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

#include "cr_base/compiler_config.h"

#include "cr_base/base_export.h"
#include "cr_base/time/time.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "cr_base/win/windows_types.h"
#elif defined(MINI_CHROMIUM_OS_POSIX)
#include <pthread.h>
#include <unistd.h>
#endif

namespace cr {

// -- PlatformThreadId ---------------------------------------------------------

// Used for logging. Always an integer value.
#if defined(MINI_CHROMIUM_OS_WIN)
typedef DWORD PlatformThreadId;
#elif defined(MINI_CHROMIUM_OS_POSIX)
typedef pid_t PlatformThreadId;
#endif

const PlatformThreadId kInvalidThreadId(0);

// -- PlatformThreadHandle -----------------------------------------------------

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


// Valid values for priority of Thread::Options and SimpleThread::Options, and
// SetCurrentThreadPriority(), listed in increasing order of importance.
enum class ThreadPriority : int {
  // Suitable for threads that shouldn't disrupt high priority work.
  BACKGROUND,
  // Default priority level.
  NORMAL,
  // Suitable for threads which generate data for the display (at ~60Hz).
  DISPLAY,
  // Suitable for low-latency, glitch-resistant audio.
  REALTIME_AUDIO,
};

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

  // Implement this interface to run code on a background thread.  Your
  // ThreadMain method will be called on the newly created thread.
  class CRBASE_EXPORT Delegate {
   public:
    // The interval at which the thread expects to have work to do. Zero if
    // unknown. (Example: audio buffer duration for real-time audio.) Is used to
    // optimize the thread real-time behavior. Is called on the newly created
    // thread before ThreadMain().
    virtual TimeDelta GetRealtimePeriod();

    virtual void ThreadMain() = 0;

   protected:
    virtual ~Delegate() = default;
  };

  // Creates a new thread.  The |stack_size| parameter can be 0 to indicate
  // that the default stack size should be used.  Upon success,
  // |*thread_handle| will be assigned a handle to the newly created thread,
  // and |delegate|'s ThreadMain method will be executed on the newly created
  // thread.
  // NOTE: When you are done with the thread handle, you must call Join to
  // release system resources associated with the thread.  You must ensure that
  // the Delegate object outlives the thread.
  static bool Create(size_t stack_size,
                     Delegate* delegate,
                     PlatformThreadHandle* thread_handle) {
    return CreateWithPriority(stack_size, delegate, thread_handle,
                              ThreadPriority::NORMAL);
  }

  // CreateWithPriority() does the same thing as Create() except the priority of
  // the thread is set based on |priority|.
  static bool CreateWithPriority(size_t stack_size, Delegate* delegate,
                                 PlatformThreadHandle* thread_handle,
                                 ThreadPriority priority);

  // CreateNonJoinable() does the same thing as Create() except the thread
  // cannot be Join()'d.  Therefore, it also does not output a
  // PlatformThreadHandle.
  static bool CreateNonJoinable(size_t stack_size, Delegate* delegate);

  // CreateNonJoinableWithPriority() does the same thing as CreateNonJoinable()
  // except the priority of the thread is set based on |priority|.
  static bool CreateNonJoinableWithPriority(size_t stack_size,
                                            Delegate* delegate,
                                            ThreadPriority priority);

  // Joins with a thread created via the Create function.  This function blocks
  // the caller until the designated thread exits.  This will invalidate
  // |thread_handle|.
  static void Join(PlatformThreadHandle thread_handle);

  // Detaches and releases the thread handle. The thread is no longer joinable
  // and |thread_handle| is invalidated after this call.
  static void Detach(PlatformThreadHandle thread_handle);

  // Toggles the current thread's priority at runtime.
  //
  // A thread may not be able to raise its priority back up after lowering it if
  // the process does not have a proper permission, e.g. CAP_SYS_NICE on Linux.
  // A thread may not be able to lower its priority back down after raising it
  // to REALTIME_AUDIO.
  //
  // This function must not be called from the main thread on Mac. This is to
  // avoid performance regressions (https://crbug.com/601270).
  //
  // Since changing other threads' priority is not permitted in favor of
  // security, this interface is restricted to change only the current thread
  // priority (https://crbug.com/399473).
  static void SetCurrentThreadPriority(ThreadPriority priority);

  static ThreadPriority GetCurrentThreadPriority();

  // Returns the default thread stack size set by chrome. If we do not
// explicitly set default size then returns 0.
  static size_t GetDefaultThreadStackSize();

#if defined(MINI_CHROMIUM_OS_LINUX)
  // Toggles a specific thread's priority at runtime. This can be used to
  // change the priority of a thread in a different process and will fail
  // if the calling process does not have proper permissions. The
  // SetCurrentThreadPriority() function above is preferred in favor of
  // security but on platforms where sandboxed processes are not allowed to
  // change priority this function exists to allow a non-sandboxed process
  // to change the priority of sandboxed threads for improved performance.
  // Warning: Don't use this for a main thread because that will change the
  // whole thread group's (i.e. process) priority.
  static void SetThreadPriority(PlatformThreadId process_id,
                                PlatformThreadId thread_id,
                                ThreadPriority priority);
#endif

 private:
  static void SetCurrentThreadPriorityImpl(ThreadPriority priority);
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_THREADING_PLATFORM_THREAD_H_
