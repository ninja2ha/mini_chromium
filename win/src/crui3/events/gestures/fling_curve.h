// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_GESTURES_FLING_CURVE_H_
#define UI_EVENTS_GESTURES_FLING_CURVE_H_

#include "crbase/time/time.h"
#include "crui/events/gesture_curve.h"
#include "crui/gfx/geometry/point_f.h"
#include "crui/gfx/geometry/vector2d_f.h"
#include "crui/base/ui_export.h"

namespace crui {

// FlingCurve can be used to scroll a UI element suitable for touch screen-based
// flings.
class CRUI_EXPORT FlingCurve : public GestureCurve {
 public:
  FlingCurve(const FlingCurve&) = delete;
  FlingCurve& operator=(const FlingCurve&) = delete;

  FlingCurve(const gfx::Vector2dF& velocity, cr::TimeTicks start_timestamp);
  ~FlingCurve() override;

  // GestureCurve implementation.
  bool ComputeScrollOffset(cr::TimeTicks time,
                           gfx::Vector2dF* offset,
                           gfx::Vector2dF* velocity) override;

  // In contrast to |ComputeScrollOffset()|, this method is stateful and
  // returns the *change* in scroll offset between successive calls.
  // Returns true as long as the curve is still active and requires additional
  // animation ticks.
  bool ComputeScrollDeltaAtTime(cr::TimeTicks current, gfx::Vector2dF* delta);

 private:
  const float curve_duration_;
  const cr::TimeTicks start_timestamp_;

  gfx::Vector2dF displacement_ratio_;
  gfx::Vector2dF cumulative_scroll_;
  cr::TimeTicks previous_timestamp_;
  float time_offset_;
  float position_offset_;
};

}  // namespace crui

#endif  // UI_EVENTS_GESTURES_FLING_CURVE_H_
