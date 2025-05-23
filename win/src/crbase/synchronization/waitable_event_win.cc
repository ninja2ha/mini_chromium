// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/synchronization/waitable_event.h"

#include <windows.h>
#include <stddef.h>

#include <algorithm>
#include <utility>

///#include "base/debug/activity_tracker.h"
#include "crbase/logging.h"
#include "crbase/numerics/safe_conversions.h"
#include "crbase/threading/thread_restrictions.h"
#include "crbase/time/time.h"

namespace cr {

WaitableEvent::WaitableEvent(ResetPolicy reset_policy,
                             InitialState initial_state)
    : handle_(CreateEvent(nullptr,
                          reset_policy == ResetPolicy::MANUAL,
                          initial_state == InitialState::SIGNALED,
                          nullptr)) {
  // We're probably going to crash anyways if this is ever NULL, so we might as
  // well make our stack reports more informative by crashing here.
  CR_CHECK(handle_.IsValid());
}

WaitableEvent::WaitableEvent(win::ScopedHandle handle)
    : handle_(std::move(handle)) {
  CR_CHECK(handle_.IsValid()) 
      << "Tried to create WaitableEvent from NULL handle";
}

WaitableEvent::~WaitableEvent() = default;

void WaitableEvent::Reset() {
  ResetEvent(handle_.Get());
}

void WaitableEvent::Signal() {
  SetEvent(handle_.Get());
}

bool WaitableEvent::IsSignaled() {
  DWORD result = WaitForSingleObject(handle_.Get(), 0);
  CR_DCHECK(result == WAIT_OBJECT_0 || result == WAIT_TIMEOUT)
      << "Unexpected WaitForSingleObject result " << result;
  return result == WAIT_OBJECT_0;
}

void WaitableEvent::Wait() {
  cr::ThreadRestrictions::AssertWaitAllowed();
  // Record the event that this thread is blocking upon (for hang diagnosis).
  ///base::debug::ScopedEventWaitActivity event_activity(this);

  DWORD result = WaitForSingleObject(handle_.Get(), INFINITE);
  // It is most unexpected that this should ever fail.  Help consumers learn
  // about it if it should ever fail.
  CR_DCHECK(WAIT_OBJECT_0 == result) << "WaitForSingleObject failed";
}

namespace {

// Helper function called from TimedWait and TimedWaitUntil.
bool WaitUntil(HANDLE handle, const TimeTicks& now, const TimeTicks& end_time) {
  TimeDelta delta = end_time - now;
  CR_DCHECK(delta > TimeDelta());

  do {
    // On Windows, waiting for less than 1 ms results in WaitForSingleObject
    // returning promptly which may result in the caller code spinning.
    // We need to ensure that we specify at least the minimally possible 1 ms
    // delay unless the initial timeout was exactly zero.
    delta = std::max(delta, TimeDelta::FromMilliseconds(1));
    // Truncate the timeout to milliseconds.
    DWORD timeout_ms = saturated_cast<DWORD>(delta.InMilliseconds());
    DWORD result = WaitForSingleObject(handle, timeout_ms);
    CR_DCHECK(result == WAIT_OBJECT_0 || result == WAIT_TIMEOUT)
        << "Unexpected WaitForSingleObject result " << result;
    switch (result) {
      case WAIT_OBJECT_0:
        return true;
      case WAIT_TIMEOUT:
        // TimedWait can time out earlier than the specified |timeout| on
        // Windows. To make this consistent with the posix implementation we
        // should guarantee that TimedWait doesn't return earlier than the
        // specified |max_time| and wait again for the remaining time.
        delta = end_time - TimeTicks::Now();
        break;
    }
  } while (delta > TimeDelta());
  return false;
}

}  // namespace

bool WaitableEvent::TimedWait(const TimeDelta& wait_delta) {
  CR_DCHECK(wait_delta >= TimeDelta());
  if (wait_delta.is_zero())
    return IsSignaled();

  cr::ThreadRestrictions::AssertWaitAllowed();
  // Record the event that this thread is blocking upon (for hang diagnosis).
  ///base::debug::ScopedEventWaitActivity event_activity(this);

  TimeTicks now(TimeTicks::Now());
  // TimeTicks takes care of overflow including the cases when wait_delta
  // is a maximum value.
  return WaitUntil(handle_.Get(), now, now + wait_delta);
}

bool WaitableEvent::TimedWaitUntil(const TimeTicks& end_time) {
  if (end_time.is_null())
    return IsSignaled();

  cr::ThreadRestrictions::AssertWaitAllowed();
  // Record the event that this thread is blocking upon (for hang diagnosis).
  ///base::debug::ScopedEventWaitActivity event_activity(this);

  TimeTicks now(TimeTicks::Now());
  if (end_time <= now)
    return IsSignaled();

  return WaitUntil(handle_.Get(), now, end_time);
}

// static
size_t WaitableEvent::WaitMany(WaitableEvent** events, size_t count) {
  CR_DCHECK(count) << "Cannot wait on no events";

  cr::ThreadRestrictions::AssertWaitAllowed();
  // Record an event (the first) that this thread is blocking upon.
  ///base::debug::ScopedEventWaitActivity event_activity(events[0]);

  HANDLE handles[MAXIMUM_WAIT_OBJECTS];
  CR_CHECK(count <= static_cast<size_t>(MAXIMUM_WAIT_OBJECTS))
      << "Can only wait on " << MAXIMUM_WAIT_OBJECTS << " with WaitMany";

  for (size_t i = 0; i < count; ++i)
    handles[i] = events[i]->handle();

  // The cast is safe because count is small - see the CHECK above.
  DWORD result =
      WaitForMultipleObjects(static_cast<DWORD>(count),
                             handles,
                             FALSE,      // don't wait for all the objects
                             INFINITE);  // no timeout
  if (result >= WAIT_OBJECT_0 + count) {
    CR_DPLOG(Fatal) << "WaitForMultipleObjects failed";
    return 0;
  }

  return result - WAIT_OBJECT_0;
}

}  // namespace cr