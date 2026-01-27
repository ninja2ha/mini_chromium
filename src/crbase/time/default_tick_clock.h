// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_TIME_DEFAULT_TICK_CLOCK_H_
#define MINI_CHROMIUM_SRC_CRBASE_TIME_DEFAULT_TICK_CLOCK_H_

#include "crbase/base_export.h"
#include "crbase/time/tick_clock.h"

namespace cr {

// DefaultTickClock is a TickClock implementation that uses TimeTicks::Now().
// This is typically used by components that expose a SetTickClockForTesting().
// Note: Overriding Time/TimeTicks altogether via
// TaskEnvironment::TimeSource::MOCK_TIME is now the preferred way of overriding
// time in unit tests. As such, there shouldn't be many new use cases for
// TickClock/DefaultTickClock anymore.
class CRBASE_EXPORT DefaultTickClock : public TickClock {
 public:
  ~DefaultTickClock() override;

  // Simply returns TimeTicks::Now().
  TimeTicks NowTicks() const override;

  // Returns a shared instance of DefaultTickClock. This is thread-safe.
  static const DefaultTickClock* GetInstance();
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_TIME_DEFAULT_TICK_CLOCK_H_
