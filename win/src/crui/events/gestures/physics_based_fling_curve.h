// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_GESTURES_PHYSICS_BASED_FLING_CURVE_H_
#define UI_EVENTS_GESTURES_PHYSICS_BASED_FLING_CURVE_H_

#include "crbase/time/time.h"
#include "crui/events/gesture_curve.h"
#include "crui/gfx/geometry/cubic_bezier.h"
#include "crui/gfx/geometry/point_f.h"
#include "crui/gfx/geometry/size.h"
#include "crui/gfx/geometry/vector2d_f.h"
#include "crui/base/ui_export.h"

namespace crui {

// PhysicsBasedFlingCurve generates animation curve, similar to
// DirectManipulation's fling curve that can be used to scroll a UI element
// suitable for touch screen-based flings.
class CRUI_EXPORT PhysicsBasedFlingCurve : public GestureCurve {
 public:
  PhysicsBasedFlingCurve(const PhysicsBasedFlingCurve&) = delete;
  PhysicsBasedFlingCurve& operator=(const PhysicsBasedFlingCurve&) = delete;

  PhysicsBasedFlingCurve(const gfx::Vector2dF& velocity,
                         cr::TimeTicks start_timestamp,
                         const gfx::Vector2dF& pixels_per_inch,
                         const gfx::Size& viewport);
  ~PhysicsBasedFlingCurve() override;

  // GestureCurve implementation.
  bool ComputeScrollOffset(cr::TimeTicks time,
                           gfx::Vector2dF* offset,
                           gfx::Vector2dF* velocity) override;

  float curve_duration() const { return curve_duration_; }
  const gfx::PointF& p1_for_testing() const { return p1_; }
  const gfx::PointF& p2_for_testing() const { return p2_; }

 private:
  // Time when fling curve is generated.
  const cr::TimeTicks start_timestamp_;
  // Cubic bezier curve control points.
  gfx::PointF p1_;
  gfx::PointF p2_;
  // Distance it can scroll with input velocity.
  const gfx::Vector2dF distance_;
  // Time in seconds, till which fling can remain active relative to
  // |start_timestamp_|.
  // TODO (sarsha): Use base::TimeDelta for |curve_duration_| once
  // crrev.com/c/1865928 is merged.
  // crbug.com/1028501
  const float curve_duration_;
  const gfx::CubicBezier bezier_;
  cr::TimeDelta previous_time_delta_;
  gfx::Vector2dF cumulative_scroll_;
  gfx::Vector2dF prev_offset_;

  float CalculateDurationAndConfigureControlPoints(
      const gfx::Vector2dF& velocity);
};

}  // namespace crui

#endif  // UI_EVENTS_GESTURES_PHYSICS_BASED_FLING_CURVE_H_