// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/events/gesture_event_details.h"

namespace crui {

GestureEventDetails::GestureEventDetails()
    : type_(ET_UNKNOWN),
      device_type_(GestureDeviceType::DEVICE_UNKNOWN),
      touch_points_(0) {}

GestureEventDetails::GestureEventDetails(crui::EventType type)
    : type_(type),
      device_type_(GestureDeviceType::DEVICE_UNKNOWN),
      touch_points_(1) {
  CR_DCHECK(type >= ET_GESTURE_TYPE_START);
  CR_DCHECK(type <= ET_GESTURE_TYPE_END);
}

GestureEventDetails::GestureEventDetails(
    crui::EventType type,
    float delta_x,
    float delta_y,
    crui::input_types::ScrollGranularity units)
    : type_(type),
      device_type_(GestureDeviceType::DEVICE_UNKNOWN),
      touch_points_(1) {
  CR_DCHECK(type >= ET_GESTURE_TYPE_START);
  CR_DCHECK(type <= ET_GESTURE_TYPE_END);
  switch (type_) {
    case crui::ET_GESTURE_SCROLL_BEGIN:
      data_.scroll_begin.x_hint = delta_x;
      data_.scroll_begin.y_hint = delta_y;
      data_.scroll_begin.delta_hint_units = units;
      break;

    case crui::ET_GESTURE_SCROLL_UPDATE:
      data_.scroll_update.x = delta_x;
      data_.scroll_update.y = delta_y;
      data_.scroll_update.delta_units = units;
      break;

    case crui::ET_SCROLL_FLING_START:
      data_.fling_velocity.x = delta_x;
      data_.fling_velocity.y = delta_y;
      break;

    case crui::ET_GESTURE_TWO_FINGER_TAP:
      data_.first_finger_enclosing_rectangle.width = delta_x;
      data_.first_finger_enclosing_rectangle.height = delta_y;
      break;

    case crui::ET_GESTURE_SWIPE:
      data_.swipe.left = delta_x < 0;
      data_.swipe.right = delta_x > 0;
      data_.swipe.up = delta_y < 0;
      data_.swipe.down = delta_y > 0;
      break;

    default:
      CR_NOTREACHED() << "Invalid event type for constructor: " << type;
  }
}

GestureEventDetails::GestureEventDetails(crui::EventType type,
                                         const GestureEventDetails& other)
    : type_(type),
      data_(other.data_),
      device_type_(other.device_type_),
      primary_pointer_type_(other.primary_pointer_type_),
      touch_points_(other.touch_points_),
      bounding_box_(other.bounding_box_) {
  CR_DCHECK(type >= ET_GESTURE_TYPE_START);
  CR_DCHECK(type <= ET_GESTURE_TYPE_END);
  switch (type) {
    case crui::ET_GESTURE_SCROLL_BEGIN:
      // Synthetic creation of SCROLL_BEGIN from PINCH_BEGIN is explicitly
      // allowed as an exception.
      if (other.type() == crui::ET_GESTURE_PINCH_BEGIN)
        break;
      CR_FALLTHROUGH;
    case crui::ET_GESTURE_SCROLL_UPDATE:
    case crui::ET_SCROLL_FLING_START:
    case crui::ET_GESTURE_SWIPE:
    case crui::ET_GESTURE_PINCH_UPDATE:
      CR_DCHECK(type == other.type()) << " - Invalid gesture conversion from "
                                      << other.type() << " to " << type;
      break;
    default:
      break;
  }
}

GestureEventDetails::Details::Details() {
  memset(this, 0, sizeof(Details));
}

}  // namespace crui
