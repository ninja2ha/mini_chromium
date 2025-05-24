// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_GESTURES_FIXED_VELOCITY_CURVE_H_
#define UI_EVENTS_GESTURES_FIXED_VELOCITY_CURVE_H_

#include "crbase/time/time.h"
#include "crui/events/gesture_curve.h"
#include "crui/gfx/geometry/point_f.h"
#include "crui/gfx/geometry/vector2d_f.h"

namespace crui {

class CRUI_EXPORT FixedVelocityCurve : public GestureCurve {
 public:
  FixedVelocityCurve(const FixedVelocityCurve&) = delete;
  FixedVelocityCurve& operator=(const FixedVelocityCurve&) = delete;

  FixedVelocityCurve(const gfx::Vector2dF& velocity,
                     cr::TimeTicks start_timestamp);
  ~FixedVelocityCurve() override;

  // GestureCurve implementation.
  bool ComputeScrollOffset(cr::TimeTicks time,
                           gfx::Vector2dF* offset,
                           gfx::Vector2dF* velocity) override;

 private:
  const gfx::Vector2dF velocity_;
  const cr::TimeTicks start_timestamp_;
};

}  // namespace crui

#endif  // UI_EVENTS_GESTURES_FIXED_VELOCITY_CURVE_H_
