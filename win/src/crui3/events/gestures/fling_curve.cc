// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/events/gestures/fling_curve.h"

#include <algorithm>
#include <cmath>

#include "crbase/logging.h"

namespace {

const float kDefaultAlpha = -5.70762e+03f;
const float kDefaultBeta = 1.72e+02f;
const float kDefaultGamma = 3.7e+00f;

inline double GetPositionAtTime(double t) {
  return kDefaultAlpha * exp(-kDefaultGamma * t) - kDefaultBeta * t -
         kDefaultAlpha;
}

inline double GetVelocityAtTime(double t) {
  return -kDefaultAlpha * kDefaultGamma * exp(-kDefaultGamma * t) -
         kDefaultBeta;
}

inline double GetTimeAtVelocity(double v) {
  return -log((v + kDefaultBeta) / (-kDefaultAlpha * kDefaultGamma)) /
         kDefaultGamma;
}

}  // namespace

namespace crui {

FlingCurve::FlingCurve(const gfx::Vector2dF& velocity,
                       cr::TimeTicks start_timestamp)
    : curve_duration_(static_cast<float>(GetTimeAtVelocity(0))),
      start_timestamp_(start_timestamp),
      previous_timestamp_(start_timestamp_),
      time_offset_(0.f),
      position_offset_(0.f) {
  CR_DCHECK(!velocity.IsZero());
  float max_start_velocity = std::max(fabs(velocity.x()), fabs(velocity.y()));
  if (max_start_velocity > static_cast<float>(GetVelocityAtTime(0.f)))
    max_start_velocity = static_cast<float>(GetVelocityAtTime(0.f));
  CR_CHECK(max_start_velocity > 0);

  displacement_ratio_ = gfx::Vector2dF(velocity.x() / max_start_velocity,
                                       velocity.y() / max_start_velocity);
  time_offset_ = static_cast<float>(GetTimeAtVelocity(max_start_velocity));
  position_offset_ = static_cast<float>(GetPositionAtTime(time_offset_));
}

FlingCurve::~FlingCurve() {
}

bool FlingCurve::ComputeScrollOffset(cr::TimeTicks time,
                                     gfx::Vector2dF* offset,
                                     gfx::Vector2dF* velocity) {
  CR_DCHECK(offset);
  CR_DCHECK(velocity);
  cr::TimeDelta elapsed_time = time - start_timestamp_;
  if (elapsed_time < cr::TimeDelta()) {
    *offset = gfx::Vector2dF();
    *velocity = gfx::Vector2dF();
    return true;
  }

  bool still_active = true;
  float scalar_offset;
  float scalar_velocity;
  double offset_time = elapsed_time.InSecondsF() + time_offset_;
  if (offset_time < curve_duration_) {
    scalar_offset = static_cast<float>(
        GetPositionAtTime(offset_time) - position_offset_);
    scalar_velocity = static_cast<float>(
        GetVelocityAtTime(offset_time));
  } else {
    scalar_offset = static_cast<float>(
        GetPositionAtTime(curve_duration_) - position_offset_);
    scalar_velocity = 0;
    still_active = false;
  }

  *offset = gfx::ScaleVector2d(displacement_ratio_, scalar_offset);
  *velocity = gfx::ScaleVector2d(displacement_ratio_, scalar_velocity);
  return still_active;
}

bool FlingCurve::ComputeScrollDeltaAtTime(cr::TimeTicks current,
                                          gfx::Vector2dF* delta) {
  CR_DCHECK(delta);
  if (current <= previous_timestamp_) {
    *delta = gfx::Vector2dF();
    return true;
  }

  previous_timestamp_ = current;

  gfx::Vector2dF offset, velocity;
  bool still_active = ComputeScrollOffset(current, &offset, &velocity);

  *delta = offset - cumulative_scroll_;
  cumulative_scroll_ = offset;
  return still_active;
}

}  // namespace crui
