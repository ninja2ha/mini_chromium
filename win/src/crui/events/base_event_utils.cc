// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/events/base_event_utils.h"

#include "crbase/logging.h"
#include "crbase/atomic/atomic_sequence_num.h"
#include "crbase/memory/lazy_instance.h"
#include "crbase/time/time.h"
#include "crui/events/event_constants.h"
#include "crui/base/build_platform.h"

namespace crui {

namespace {

#if defined(MINI_CHROMIUM_OS_MACOSX)
// Alt modifier is used to input extended characters on Mac.
const int kSystemKeyModifierMask = EF_COMMAND_DOWN;
#else
const int kSystemKeyModifierMask = EF_ALT_DOWN;
#endif  // !defined(OS_CHROMEOS) && !defined(OS_MACOSX)

}  // namespace

cr::AtomicSequenceNumber g_next_event_id;

uint32_t GetNextTouchEventId() {
  // Set the first touch event ID to 1 because we set id to 0 for other types
  // of events.
  uint32_t id = g_next_event_id.GetNext();
  if (id == 0)
    id = g_next_event_id.GetNext();
  CR_DCHECK(0U != id);
  return id;
}

bool IsSystemKeyModifier(int flags) {
  // AltGr modifier is used to type alternative keys on certain keyboard layouts
  // so we don't consider keys with the AltGr modifier as a system key.
  return (kSystemKeyModifierMask & flags) != 0 &&
         (EF_ALTGR_DOWN & flags) == 0;
}

cr::LazyInstance<const cr::TickClock*>::Leaky g_tick_clock =
    LAZY_INSTANCE_INITIALIZER;

cr::TimeTicks EventTimeForNow() {
  return g_tick_clock.Get() ? g_tick_clock.Get()->NowTicks()
                            : cr::TimeTicks::Now();
}

void SetEventTickClockForTesting(const cr::TickClock* tick_clock) {
  g_tick_clock.Get() = tick_clock;
}

double EventTimeStampToSeconds(cr::TimeTicks time_stamp) {
  return (time_stamp - cr::TimeTicks()).InSecondsF();
}

cr::TimeTicks EventTimeStampFromSeconds(double time_stamp_seconds) {
  return cr::TimeTicks() + cr::TimeDelta::FromSecondsD(time_stamp_seconds);
}

}  // namespace ui
