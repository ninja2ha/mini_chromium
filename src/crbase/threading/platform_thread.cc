// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 91.0.4472.169

#include "crbase/threading/platform_thread.h"

#include "crbase/time/time_override.h"
#include "cr/build_config.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "crbase/win/windows_types.h"
#elif defined(MINI_CHROMIUM_OS_POSIX)
#include <pthread.h>
#include <sched.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#if defined(MINI_CHROMIUM_OS_LINUX)
#include <sys/syscall.h>
#include "crbase/memory/no_destructor.h"
#endif

namespace cr {

namespace {

#if defined(MINI_CHROMIUM_OS_LINUX)
// Store the thread ids in local storage since calling the SWI can be
// expensive and PlatformThread::CurrentId is used liberally. Clear
// the stored value after a fork() because forking changes the thread
// id. Forking without going through fork() (e.g. clone()) is not
// supported, but there is no known usage. Using thread_local is
// fine here (despite being banned) since it is going to be allowed
// but is blocked on a clang bug for Mac (https://crbug.com/829078)
// and we can't use ThreadLocalStorage because of re-entrancy due to
// CR_CHECK/CR_DCHECKs.
thread_local pid_t g_thread_id = -1;

void CrClearTidCache() {
  g_thread_id = -1;
}

class InitAtFork {
 public:
  InitAtFork() { pthread_atfork(nullptr, nullptr, CrClearTidCache); }
};
#endif  // defined(MINI_CHROMIUM_OS_LINUX)

}  // namespace

// static
PlatformThreadId PlatformThread::CurrentId() {
#if defined(MINI_CHROMIUM_OS_WIN)
  return ::GetCurrentThreadId();
#elif defined(MINI_CHROMIUM_OS_LINUX)
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
#else
  CR_NOTIMPLEMENTED();
#endif
}

// static
PlatformThreadRef PlatformThread::CurrentRef() {
#if defined(MINI_CHROMIUM_OS_WIN)
  return PlatformThreadRef(::GetCurrentThreadId());
#elif defined(MINI_CHROMIUM_OS_POSIX)
  return PlatformThreadHandle(pthread_self());
#else
  CR_NOTIMPLEMENTED();
#endif
}

// static
PlatformThreadHandle PlatformThread::CurrentHandle() {
#if defined(MINI_CHROMIUM_OS_WIN)
  return PlatformThreadHandle(::GetCurrentThread());
#elif defined(MINI_CHROMIUM_OS_POSIX)
  return PlatformThreadHandle(pthread_self());
#else
  CR_NOTIMPLEMENTED();
#endif
}

// static
void PlatformThread::YieldCurrentThread() {
#if defined(MINI_CHROMIUM_OS_WIN)
  ::Sleep(0);
#elif defined(MINI_CHROMIUM_OS_POSIX)
  sched_yield();
#else
  CR_NOTIMPLEMENTED();
#endif
}

// static
void PlatformThread::Sleep(TimeDelta duration) {
#if defined(MINI_CHROMIUM_OS_WIN)
  // When measured with a high resolution clock, Sleep() sometimes returns much
  // too early. We may need to call it repeatedly to get the desired duration.
  // PlatformThread::Sleep doesn't support mock-time, so this always uses
  // real-time.
  const TimeTicks end = subtle::TimeTicksNowIgnoringOverride() + duration;
  for (TimeTicks now = subtle::TimeTicksNowIgnoringOverride(); now < end;
    now = subtle::TimeTicksNowIgnoringOverride()) {
    ::Sleep(static_cast<DWORD>((end - now).InMillisecondsRoundedUp()));
  }
#elif defined(MINI_CHROMIUM_OS_POSIX)
  struct timespec sleep_time, remaining;
  // Break the duration into seconds and nanoseconds.
  // NOTE: TimeDelta's microseconds are int64s while timespec's
  // nanoseconds are longs, so this unpacking must prevent overflow.
  sleep_time.tv_sec = duration.InSeconds();
  duration -= TimeDelta::FromSeconds(sleep_time.tv_sec);
  sleep_time.tv_nsec = duration.InMicroseconds() * 1000;  // nanoseconds

  while (nanosleep(&sleep_time, &remaining) == -1 && errno == EINTR)
    sleep_time = remaining;
#else
  CR_NOTIMPLEMENTED();
#endif
}

}  // namespae cr