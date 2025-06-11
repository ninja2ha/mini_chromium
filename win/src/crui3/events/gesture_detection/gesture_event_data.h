// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_GESTURE_DETECTION_GESTURE_EVENT_DATA_H_
#define UI_EVENTS_GESTURE_DETECTION_GESTURE_EVENT_DATA_H_

#include <stddef.h>

#include "crbase/time/time.h"
#include "crui/events/event_constants.h"
#include "crui/base/ui_export.h"
#include "crui/events/gesture_detection/motion_event.h"
#include "crui/events/gesture_event_details.h"

namespace crui {

class GestureEventDataPacket;

struct CRUI_EXPORT GestureEventData {
  GestureEventData(const GestureEventDetails&,
                   int motion_event_id,
                   MotionEvent::ToolType primary_tool_type,
                   cr::TimeTicks time,
                   float x,
                   float y,
                   float raw_x,
                   float raw_y,
                   size_t touch_point_count,
                   const gfx::RectF& bounding_box,
                   int flags,
                   uint32_t unique_touch_event_id);
  GestureEventData(EventType type, const GestureEventData&);
  GestureEventData(const GestureEventData& other);

  EventType type() const { return details.type(); }

  GestureEventDetails details;
  int motion_event_id;
  // The tool type for the first touch point in the gesture.
  MotionEvent::ToolType primary_tool_type;
  cr::TimeTicks time;
  float x;
  float y;
  float raw_x;
  float raw_y;
  int flags;

  // The unique id of the touch event that released the gesture event. This
  // field gets a non-zero from the corresponding field in
  // GestureEventDataPacket at the moment the gesture is pushed into the packet.
  uint32_t unique_touch_event_id;

 private:
  friend class GestureEventDataPacket;

  // Initializes type to GESTURE_TYPE_INVALID.
  GestureEventData();
};

}  //  namespace crui

#endif  // UI_EVENTS_GESTURE_DETECTION_GESTURE_EVENT_DATA_H_
