// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/threading/platform_thread.h"

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

#include "crbase/logging/logging.h"
#include "crbase/memory/no_destructor.h"
///#include "base/threading/scoped_blocking_call.h"
#include "crbase/threading/thread_id_name_manager.h"
#include "cr/build_config.h"

///#if !defined(OS_APPLE) && !defined(OS_FUCHSIA) && !defined(OS_NACL)
///#include "base/posix/can_lower_nice_to.h"
///#endif

#if defined(MINI_CHROMIUM_OS_LINUX)
#include <sys/syscall.h>
#endif


namespace cr {

void InitThreading();
void TerminateOnThread();
size_t GetDefaultThreadStackSize(const pthread_attr_t& attributes);

namespace {

struct ThreadParams {
  ThreadParams()
      : delegate(nullptr), joinable(false), priority(ThreadPriority::NORMAL) {}

  PlatformThread::Delegate* delegate;
  bool joinable;
  ThreadPriority priority;
};

void* ThreadFunc(void* params) {
  PlatformThread::Delegate* delegate = nullptr;

  {
    std::unique_ptr<ThreadParams> thread_params(
        static_cast<ThreadParams*>(params));

    delegate = thread_params->delegate;
    ///if (!thread_params->joinable)
    ///  cr::ThreadRestrictions::SetSingletonAllowed(false);

    // Threads on linux/android may inherit their priority from the thread
    // where they were created. This explicitly sets the priority of all new
    // threads.
    ///PlatformThread::SetCurrentThreadPriority(thread_params->priority);
  }

  ThreadIdNameManager::GetInstance()->RegisterThread(
      PlatformThread::CurrentHandle().platform_handle(),
      PlatformThread::CurrentId());

  delegate->ThreadMain();

  ThreadIdNameManager::GetInstance()->RemoveName(
      PlatformThread::CurrentHandle().platform_handle(),
      PlatformThread::CurrentId());

  cr::TerminateOnThread();
  return nullptr;
}

bool CreateThread(size_t stack_size,
                  bool joinable,
                  PlatformThread::Delegate* delegate,
                  PlatformThreadHandle* thread_handle,
                  ThreadPriority priority) {
  CR_DCHECK(thread_handle);
  cr::InitThreading();

  pthread_attr_t attributes;
  pthread_attr_init(&attributes);

  // Pthreads are joinable by default, so only specify the detached
  // attribute if the thread should be non-joinable.
  if (!joinable)
    pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);

  // Get a better default if available.
  if (stack_size == 0)
    stack_size = cr::GetDefaultThreadStackSize(attributes);

  if (stack_size > 0)
    pthread_attr_setstacksize(&attributes, stack_size);

  std::unique_ptr<ThreadParams> params(new ThreadParams);
  params->delegate = delegate;
  params->joinable = joinable;
  params->priority = priority;

  pthread_t handle;
  int err = pthread_create(&handle, &attributes, ThreadFunc, params.get());
  bool success = !err;
  if (success) {
    // ThreadParams should be deleted on the created thread after used.
    ignore_result(params.release());
  } else {
    // Value of |handle| is undefined if pthread_create fails.
    handle = 0;
    errno = err;
    CR_PLOG(Error) << "pthread_create";
  }
  *thread_handle = PlatformThreadHandle(handle);

  pthread_attr_destroy(&attributes);

  return success;
}

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

class InitAtFork {
 public:
  InitAtFork() { pthread_atfork(nullptr, nullptr, internal::ClearTidCache); }
};

#endif  // defined(MINI_CHROMIUM_OS_LINUX)

}  // namespace

#if defined(MINI_CHROMIUM_OS_LINUX)

namespace internal {

void ClearTidCache() {
  g_thread_id = -1;
}

}  // namespace internal

#endif  // defined(MINI_CHROMIUM_OS_LINUX)

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
PlatformThreadRef PlatformThread::CurrentRef() {
  return PlatformThreadRef(pthread_self());
}

// static
PlatformThreadHandle PlatformThread::CurrentHandle() {
  return PlatformThreadHandle(pthread_self());
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

// static
void PlatformThread::SetName(const std::string& name) {
  ThreadIdNameManager::GetInstance()->SetName(name);

  // On linux we can get the thread names to show up in the debugger by setting
  // the process name for the LWP.  We don't want to do this for the main
  // thread because that would rename the process, causing tools like killall
  // to stop working.
  if (PlatformThread::CurrentId() == getpid())
    return;

  // http://0pointer.de/blog/projects/name-your-threads.html
  // Set the name for the LWP (which gets truncated to 15 characters).
  // Note that glibc also has a 'pthread_setname_np' api, but it may not be
  // available everywhere and it's only benefit over using prctl directly is
  // that it can set the name of threads other than the current thread.
  int err = prctl(PR_SET_NAME, name.c_str());
  // We expect EPERM failures in sandboxed processes, just ignore those.
  if (err < 0 && errno != EPERM)
    CR_DPLOG(Error) << "prctl(PR_SET_NAME)";
}

// static
const char* PlatformThread::GetName() {
  return ThreadIdNameManager::GetInstance()->GetName(CurrentId());
}

// static
bool PlatformThread::CreateWithPriority(size_t stack_size, Delegate* delegate,
                                        PlatformThreadHandle* thread_handle,
                                        ThreadPriority priority) {
  return CreateThread(stack_size, true /* joinable thread */, delegate,
                      thread_handle, priority);
}

// static
bool PlatformThread::CreateNonJoinable(size_t stack_size, Delegate* delegate) {
  return CreateNonJoinableWithPriority(stack_size, delegate,
                                       ThreadPriority::NORMAL);
}

// static
bool PlatformThread::CreateNonJoinableWithPriority(size_t stack_size,
                                                   Delegate* delegate,
                                                   ThreadPriority priority) {
  PlatformThreadHandle unused;

  bool result = CreateThread(stack_size, false /* non-joinable thread */,
                             delegate, &unused, priority);
  return result;
}

// static
void PlatformThread::Join(PlatformThreadHandle thread_handle) {
  // Joining another thread may block the current thread for a long time, since
  // the thread referred to by |thread_handle| may still be running long-lived /
  // blocking tasks.
  ///cr::internal::ScopedBlockingCallWithBaseSyncPrimitives scoped_blocking_call(
  ///    FROM_HERE, base::BlockingType::MAY_BLOCK);
  CR_CHECK(0 == pthread_join(thread_handle.platform_handle(), nullptr));
}

// static
void PlatformThread::Detach(PlatformThreadHandle thread_handle) {
  CR_CHECK(0 == pthread_detach(thread_handle.platform_handle()));
}

// static
size_t PlatformThread::GetDefaultThreadStackSize() {
  pthread_attr_t attributes;
  pthread_attr_init(&attributes);
  return cr::GetDefaultThreadStackSize(attributes);
}

}  // namespace cr