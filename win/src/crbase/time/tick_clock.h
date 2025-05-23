// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_TIME_TICK_CLOCK_H_
#define MINI_CHROMIUM_SRC_CRBASE_TIME_TICK_CLOCK_H_

#include "crbase/base_export.h"
#include "crbase/time/time.h"

namespace cr {

// A TickClock is an interface for objects that vend TimeTicks.  It is
// intended to be able to test the behavior of classes with respect to
// non-decreasing time.
//
// See DefaultTickClock (base/time/default_tick_clock.h) for the default
// implementation that simply uses TimeTicks::Now().
//
// (Other implementations that use TimeTicks::NowFromSystemTime() should
// be added as needed.)
//
// See SimpleTestTickClock (base/test/simple_test_tick_clock.h) for a
// simple test implementation.
//
// See Clock (base/time/clock.h) for the equivalent interface for Times.
class CRBASE_EXPORT TickClock {
 public:
  virtual ~TickClock();

  // NowTicks() must be safe to call from any thread.  The caller may
  // assume that NowTicks() is monotonic (but not strictly monotonic).
  // In other words, the returned TimeTicks will never decrease with
  // time, although they might "stand still".
  virtual TimeTicks NowTicks() const = 0;
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_TIME_TICK_CLOCK_H_