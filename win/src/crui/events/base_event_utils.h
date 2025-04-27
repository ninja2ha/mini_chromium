// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_BASE_EVENT_UTILS_H_
#define UI_EVENTS_BASE_EVENT_UTILS_H_

#include <memory>
#include <stdint.h>

#include "crbase/time/tick_clock.h"
#include "crui/base/ui_export.h"

namespace cr {
class TimeTicks;
}  // namespace cr

// Common functions to be used for all platforms.
namespace crui {

// Generate an unique identifier for events.
CRUI_EXPORT uint32_t GetNextTouchEventId();

// Checks if |flags| contains system key modifiers.
CRUI_EXPORT bool IsSystemKeyModifier(int flags);

// Create a timestamp based on the current time.
CRUI_EXPORT cr::TimeTicks EventTimeForNow();

// Overrides the clock used by EventTimeForNow for testing.
// This doesn't take the ownership of the clock.
CRUI_EXPORT void SetEventTickClockForTesting(
    const cr::TickClock* tick_clock);

// Converts an event timestamp ticks to seconds (floating point representation).
// WARNING: This should only be used when interfacing with platform code that
// does not use base::Time* types.
CRUI_EXPORT double EventTimeStampToSeconds(cr::TimeTicks time_stamp);

// Converts an event timestamp in seconds to TimeTicks.
// WARNING: This should only be used when interfacing with platform code that
// does not use base::Time* types.
CRUI_EXPORT cr::TimeTicks EventTimeStampFromSeconds(
    double time_stamp_seconds);

}  // namespace crui

#endif  // UI_EVENTS_BASE_EVENT_UTILS_H_
