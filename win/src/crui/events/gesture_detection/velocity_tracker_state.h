// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_GESTURE_DETECTION_VELOCITY_TRACKER_STATE_H_
#define UI_EVENTS_GESTURE_DETECTION_VELOCITY_TRACKER_STATE_H_

#include <stdint.h>

#include "crui/events/gesture_detection/bitset_32.h"
#include "crui/base/ui_export.h"
#include "crui/events/gesture_detection/velocity_tracker.h"

namespace crui {

class MotionEvent;

// Port of VelocityTrackerState from Android
// * platform/frameworks/base/core/jni/android_view_VelocityTracker.cpp
// * Change-Id: I3517881b87b47dcc209d80dbd0ac6b5cf29a766f
// * Please update the Change-Id as upstream Android changes are pulled.
class CRUI_EXPORT VelocityTrackerState {
 public:
  VelocityTrackerState(const VelocityTrackerState&) = delete;
  VelocityTrackerState& operator=(const VelocityTrackerState&) = delete;

  explicit VelocityTrackerState(VelocityTracker::Strategy strategy);
  ~VelocityTrackerState();

  void Clear();
  void AddMovement(const MotionEvent& event);
  void ComputeCurrentVelocity(int32_t units, float max_velocity);
  float GetXVelocity(int32_t id) const;
  float GetYVelocity(int32_t id) const;

 private:
  struct Velocity {
    float vx, vy;
  };

  void GetVelocity(int32_t id, float* out_vx, float* out_vy) const;

  VelocityTracker velocity_tracker_;
  int32_t active_pointer_id_;
  BitSet32 calculated_id_bits_;
  Velocity calculated_velocity_[VelocityTracker::MAX_POINTERS];
};

}  // namespace crui

#endif  // UI_EVENTS_GESTURE_DETECTION_VELOCITY_TRACKER_STATE_H_
