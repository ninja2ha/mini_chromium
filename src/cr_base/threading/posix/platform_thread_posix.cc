// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_base/threading/platform_thread.h"

#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stddef.h>
#include <stdint.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <unistd.h>

#include <memory>

#include "cr_base/compiler_config.h"

#include "cr_base/stl_util.h"
#include "cr_base/logging/logging.h"
#include "cr_base/memory/no_destructor.h"
///#include "crbase/threading/thread_id_name_manager.h"

///#if !defined(OS_APPLE) && !defined(OS_FUCHSIA) && !defined(OS_NACL)
///#include "base/posix/can_lower_nice_to.h"
///#endif

#if defined(MINI_CHROMIUM_OS_LINUX)
#include <sys/syscall.h>
#include <sys/prctl.h>
#elif defined(MINI_CHROMIUM_OS_BSD)
#include <pthread_np.h>
#endif


namespace cr {

#if defined(MINI_CHROMIUM_OS_LINUX)
// Store the thread ids in local storage since calling the SWI can be
// expensive and PlatformThread::CurrentId is used liberally. Clear
// the stored value after a fork() because forking changes the thread
// id. Forking without going through fork() (e.g. clone()) is not
// supported, but there is no known usage. Using thread_local is
// fine here (despite being banned) since it is going to be allowed
// but is blocked on a clang bug for Mac (https://crbug.com/829078)
// and we can't use ThreadLocalStorage because of re-entrancy due to
// CHECK/DCHECKs.
thread_local pid_t g_thread_id = -1;

void ClearTidCache() {
  g_thread_id = -1;
}

class InitAtFork {
 public:
  InitAtFork() { pthread_atfork(nullptr, nullptr, ClearTidCache); }
};

#endif  // defined(MINI_CHROMIUM_OS_LINUX)

}  // namespace

// -- PlatformThreadHandle -----------------------------------------------------

// static
PlatformThreadHandle PlatformThread::CurrentHandle() {
  return PlatformThreadHandle(pthread_self());
}

// -- PlatformThread -----------------------------------------------------------

// static
PlatformThreadId PlatformThread::CurrentId() {
  // Pthreads doesn't have the concept of a thread ID, so we have to reach down
  // into the kernel.
#if defined(MINI_CHROMIUM_OS_LINUX)
  static NoDestructor<InitAtFork> init_at_fork;
  if (g_thread_id == -1) {
    g_thread_id = syscall(__NR_gettid);
  } else {
    CR_DCHECK(g_thread_id == syscall(__NR_gettid))
        << "Thread id stored in TLS is different from thread id returned by "
           "the system. It is likely that the process was forked without going "
           "through fork().";
  }
  return g_thread_id;
#elif defined(MINI_CHROMIUM_OS_POSIX)
  return reinterpret_cast<int64_t>(pthread_self());
#endif
}

// static
void PlatformThread::YieldCurrentThread() {
  sched_yield();
}

// static
void PlatformThread::Sleep(TimeDelta duration) {
  struct timespec sleep_time, remaining;

  // Break the duration into seconds and nanoseconds.
  // NOTE: TimeDelta's microseconds are int64s while timespec's
  // nanoseconds are longs, so this unpacking must prevent overflow.
  sleep_time.tv_sec = duration.InSeconds();
  duration -= TimeDelta::FromSeconds(sleep_time.tv_sec);
  sleep_time.tv_nsec = duration.InMicroseconds() * 1000;  // nanoseconds

  while (nanosleep(&sleep_time, &remaining) == -1 && errno == EINTR)
    sleep_time = remaining;
}

}  // namespace cr