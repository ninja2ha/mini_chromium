// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_base/threading/platform_thread.h"

#include <stddef.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif
// Fix error with vs2017_xp
typedef struct IUnknown IUnknown;
#include <windows.h>

#include "cr_base/logging/logging.h"

namespace cr {

// -- PlatformThreadHandle -----------------------------------------------------

// static 
PlatformThreadHandle PlatformThreadHandle::Current() {
  return PlatformThreadHandle(::GetCurrentThread());
}

// -- PlatformThread -----------------------------------------------------------

// static
PlatformThreadId PlatformThread::CurrentId() {
  return ::GetCurrentThreadId();
}

// static
void PlatformThread::YieldCurrentThread() {
  ::Sleep(0);
}

// static
void PlatformThread::Sleep(TimeDelta duration) {
  // When measured with a high resolution clock, Sleep() sometimes returns much
  // too early. We may need to call it repeatedly to get the desired duration.
  // PlatformThread::Sleep doesn't support mock-time, so this always uses
  // real-time.
  const TimeTicks end = TimeTicks::Now() + duration;
  for (TimeTicks now = TimeTicks::Now(); now < end; now = TimeTicks::Now()) {
    ::Sleep(static_cast<DWORD>((end - now).InMillisecondsRoundedUp()));
  }
}

}  // namespace cr