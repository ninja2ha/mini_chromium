// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_GESTURE_DETECTION_SNAP_SCROLL_CONTROLLER_H_
#define UI_EVENTS_GESTURE_DETECTION_SNAP_SCROLL_CONTROLLER_H_

#include "crui/base/ui_export.h"
#include "crui/gfx/geometry/point_f.h"
#include "crui/gfx/geometry/size_f.h"
#include "crui/gfx/geometry/vector2d_f.h"

namespace crui {

class MotionEvent;

// Port of SnapScrollController.java from Chromium
// Controls the scroll snapping behavior based on scroll updates.
class CRUI_EXPORT SnapScrollController {
 public:
  SnapScrollController(const SnapScrollController&) = delete;
  SnapScrollController& operator=(const SnapScrollController&) = delete;

  SnapScrollController(float snap_bound, const gfx::SizeF& display_size);
  ~SnapScrollController();

  // Sets the snap scroll mode based on the event type.
  void SetSnapScrollMode(const MotionEvent& event,
                         bool is_scale_gesture_detection_in_progress);

  // Updates the snap scroll mode based on the given X and Y distance to be
  // moved on scroll.  If the scroll update is above a threshold, the snapping
  // behavior is reset.
  void UpdateSnapScrollMode(float distance_x, float distance_y);

  bool IsSnapVertical() const;
  bool IsSnapHorizontal() const;
  bool IsSnappingScrolls() const;

 private:
  enum SnapMode { SNAP_NONE, SNAP_PENDING, SNAP_HORIZ, SNAP_VERT };

  const float snap_bound_;
  const float channel_distance_;
  SnapMode mode_;
  gfx::PointF down_position_;
  gfx::Vector2dF accumulated_distance_;
};

}  // namespace crui

#endif  // UI_EVENTS_GESTURE_DETECTION_SNAP_SCROLL_CONTROLLER_H_
