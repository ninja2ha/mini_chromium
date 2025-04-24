// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_CHROMIUM_SRC_CRBASE_TIMER_ELAPSED_TIMER_H_
#define MINI_CHROMIUM_SRC_CRBASE_TIMER_ELAPSED_TIMER_H_

#include "crbase/base_export.h"
#include "crbase/time/time.h"

namespace cr {

// A simple wrapper around TimeTicks::Now().
class CRBASE_EXPORT ElapsedTimer {
 public:
  ElapsedTimer(const ElapsedTimer&) = delete;;
  ElapsedTimer& operator=(const ElapsedTimer&) = delete;;

  ElapsedTimer();
  ElapsedTimer(ElapsedTimer&& other);

  void operator=(ElapsedTimer&& other);

  // Returns the time elapsed since object construction.
  TimeDelta Elapsed() const;

  // Returns the timestamp of the creation of this timer.
  TimeTicks Begin() const { return begin_; }

 private:
  TimeTicks begin_;
};

}  // namespace cr

#endif  // MINI_CHROMIUM_SRC_CRBASE_TIMER_ELAPSED_TIMER_H_