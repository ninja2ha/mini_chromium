// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crbase/timer/elapsed_timer.h"

namespace cr {

ElapsedTimer::ElapsedTimer() {
  begin_ = TimeTicks::Now();
}

ElapsedTimer::ElapsedTimer(ElapsedTimer&& other) {
  begin_ = other.begin_;
}

void ElapsedTimer::operator=(ElapsedTimer&& other) {
  begin_ = other.begin_;
}

TimeDelta ElapsedTimer::Elapsed() const {
  return TimeTicks::Now() - begin_;
}

}  // namespace cr