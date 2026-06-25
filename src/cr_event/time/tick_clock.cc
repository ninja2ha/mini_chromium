// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cr_event/time/tick_clock.h"

#include "cr_base/memory/no_destructor.h"

namespace cr {

namespace {
// DefaultTickClock is a TickClock implementation that uses TimeTicks::Now().
// This is typically used by components that expose a SetTickClockForTesting().
// Note: Overriding Time/TimeTicks altogether via
// TaskEnvironment::TimeSource::MOCK_TIME is now the preferred way of overriding
// time in unit tests. As such, there shouldn't be many new use cases for
// TickClock/DefaultTickClock anymore.
class DefaultTickClock : public TickClock {
 public:
  ~DefaultTickClock() override = default;

  // Simply returns TimeTicks::Now().
  TimeTicks NowTicks() const override {
    return TimeTicks::Now();
  }
};

}  // namesapce

TickClock::~TickClock() = default;

// static
const TickClock* TickClock::GetDefault() {
  static const cr::NoDestructor<DefaultTickClock> default_tick_clock;
  return default_tick_clock.get();
}

}  // namespace cr
