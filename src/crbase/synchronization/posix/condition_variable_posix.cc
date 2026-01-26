// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// * VERSION: 61.0.3163.141

#include "crbase/synchronization/condition_variable.h"

#include <errno.h>
#include <stdint.h>
#include <sys/time.h>

#include "crbase/logging/logging.h"
#include "crbase/synchronization/lock.h"
#include "crbase/time/time.h"
#include "cr/build_config.h"

namespace cr {

ConditionVariable::ConditionVariable(Lock* user_lock)
    : user_mutex_(user_lock->lock_.native_handle())
#if CR_DCHECK_IS_ON()
  , user_lock_(user_lock)
#endif
{
  int rv = 0;
  // http://crbug.com/293736
  // NaCl doesn't support monotonic clock based absolute deadlines.
  // On older Android platform versions, it's supported through the
  // non-standard pthread_cond_timedwait_monotonic_np. Newer platform
  // versions have pthread_condattr_setclock.
  // Mac can use relative time deadlines.
#if defined(MINI_CHROMIUM_OS_MACOSX)
  rv = pthread_cond_init(&condition_, NULL);
#else
  pthread_condattr_t attrs;
  rv = pthread_condattr_init(&attrs);
  CR_DCHECK(0 == rv);
  pthread_condattr_setclock(&attrs, CLOCK_MONOTONIC);
  rv = pthread_cond_init(&condition_, &attrs);
  pthread_condattr_destroy(&attrs);
#endif
  CR_DCHECK(0 == rv);
}

ConditionVariable::~ConditionVariable() {
#if defined(MINI_CHROMIUM_OS_MACOSX)
  // This hack is necessary to avoid a fatal pthreads subsystem bug in the
  // Darwin kernel. http://crbug.com/517681.
  {
    cr::Lock lock;
    cr::AutoLock l(lock);
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 1;
    pthread_cond_timedwait_relative_np(&condition_, lock.lock_.native_handle(),
                                       &ts);
  }
#endif

  int rv = pthread_cond_destroy(&condition_);
  CR_DCHECK(0 == rv);
}

void ConditionVariable::Wait() {
#if CR_DCHECK_IS_ON()
  user_lock_->CheckHeldAndUnmark();
#endif
  int rv = pthread_cond_wait(&condition_, user_mutex_);
  CR_DCHECK(0 == rv);
#if CR_DCHECK_IS_ON()
  user_lock_->CheckUnheldAndMark();
#endif
}

void ConditionVariable::TimedWait(const TimeDelta& max_time) {
  int64_t usecs = max_time.InMicroseconds();
  struct timespec relative_time;
  relative_time.tv_sec = usecs / Time::kMicrosecondsPerSecond;
  relative_time.tv_nsec =
      (usecs % Time::kMicrosecondsPerSecond) * Time::kNanosecondsPerMicrosecond;

#if CR_DCHECK_IS_ON()
  user_lock_->CheckHeldAndUnmark();
#endif

#if defined(MINI_CHROMIUM_OS_MACOSX)
  int rv = pthread_cond_timedwait_relative_np(
      &condition_, user_mutex_, &relative_time);
#else
  // The timeout argument to pthread_cond_timedwait is in absolute time.
  struct timespec absolute_time;
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  absolute_time.tv_sec = now.tv_sec;
  absolute_time.tv_nsec = now.tv_nsec;

  absolute_time.tv_sec += relative_time.tv_sec;
  absolute_time.tv_nsec += relative_time.tv_nsec;
  absolute_time.tv_sec += absolute_time.tv_nsec / Time::kNanosecondsPerSecond;
  absolute_time.tv_nsec %= Time::kNanosecondsPerSecond;
  CR_DCHECK(absolute_time.tv_sec >= now.tv_sec);  // Overflow paranoia
  int rv = pthread_cond_timedwait(&condition_, user_mutex_, &absolute_time);
#endif  // MINI_CHROMIUM_OS_MACOSX

  CR_DCHECK(rv == 0 || rv == ETIMEDOUT);
#if CR_DCHECK_IS_ON()
  user_lock_->CheckUnheldAndMark();
#endif
}

void ConditionVariable::Broadcast() {
  int rv = pthread_cond_broadcast(&condition_);
  CR_DCHECK(0 == rv);
}

void ConditionVariable::Signal() {
  int rv = pthread_cond_signal(&condition_);
  CR_DCHECK(0 == rv);
}

}  // namespace cr