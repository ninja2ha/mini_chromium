// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/events/gesture_detection/gesture_event_data_packet.h"

#include "crbase/logging.h"
#include "crui/events/gesture_detection/motion_event.h"

namespace crui {
namespace {

GestureEventDataPacket::GestureSource ToGestureSource(
    const crui::MotionEvent& event) {
  switch (event.GetAction()) {
    case crui::MotionEvent::Action::DOWN:
      return GestureEventDataPacket::TOUCH_SEQUENCE_START;
    case crui::MotionEvent::Action::UP:
      return GestureEventDataPacket::TOUCH_SEQUENCE_END;
    case crui::MotionEvent::Action::MOVE:
      return GestureEventDataPacket::TOUCH_MOVE;
    case crui::MotionEvent::Action::CANCEL:
      return GestureEventDataPacket::TOUCH_SEQUENCE_CANCEL;
    case crui::MotionEvent::Action::POINTER_DOWN:
      return GestureEventDataPacket::TOUCH_START;
    case crui::MotionEvent::Action::POINTER_UP:
      return GestureEventDataPacket::TOUCH_END;
    case crui::MotionEvent::Action::NONE:
    case crui::MotionEvent::Action::HOVER_ENTER:
    case crui::MotionEvent::Action::HOVER_EXIT:
    case crui::MotionEvent::Action::HOVER_MOVE:
    case crui::MotionEvent::Action::BUTTON_PRESS:
    case crui::MotionEvent::Action::BUTTON_RELEASE:
      CR_NOTREACHED();
      return GestureEventDataPacket::INVALID;
  }
  CR_NOTREACHED();
  return GestureEventDataPacket::INVALID;
}

}  // namespace

GestureEventDataPacket::GestureEventDataPacket()
    : gesture_source_(UNDEFINED),
      ack_state_(AckState::PENDING),
      unique_touch_event_id_(0) {
}

GestureEventDataPacket::GestureEventDataPacket(
    cr::TimeTicks timestamp,
    GestureSource source,
    const gfx::PointF& touch_location,
    const gfx::PointF& raw_touch_location,
    uint32_t unique_touch_event_id)
    : timestamp_(timestamp),
      touch_location_(touch_location),
      raw_touch_location_(raw_touch_location),
      gesture_source_(source),
      ack_state_(AckState::PENDING),
      unique_touch_event_id_(unique_touch_event_id) {
  CR_DCHECK(gesture_source_ != UNDEFINED);
}

GestureEventDataPacket::GestureEventDataPacket(
    const GestureEventDataPacket& other)
    : timestamp_(other.timestamp_),
      gestures_(other.gestures_),
      touch_location_(other.touch_location_),
      raw_touch_location_(other.raw_touch_location_),
      gesture_source_(other.gesture_source_),
      ack_state_(other.ack_state_),
      unique_touch_event_id_(other.unique_touch_event_id_) {}

GestureEventDataPacket::~GestureEventDataPacket() {
}

GestureEventDataPacket& GestureEventDataPacket::operator=(
    const GestureEventDataPacket& other) {
  timestamp_ = other.timestamp_;
  gesture_source_ = other.gesture_source_;
  touch_location_ = other.touch_location_;
  raw_touch_location_ = other.raw_touch_location_;
  gestures_ = other.gestures_;
  ack_state_ = other.ack_state_;
  unique_touch_event_id_ = other.unique_touch_event_id_;
  return *this;
}

void GestureEventDataPacket::Push(const GestureEventData& original_gesture) {
  CR_DCHECK(ET_UNKNOWN != original_gesture.type());
  GestureEventData gesture(original_gesture);
  gesture.unique_touch_event_id = unique_touch_event_id_;
  gestures_->push_back(gesture);
}

GestureEventDataPacket GestureEventDataPacket::FromTouch(
    const crui::MotionEvent& touch) {
  return GestureEventDataPacket(touch.GetEventTime(), ToGestureSource(touch),
                                gfx::PointF(touch.GetX(), touch.GetY()),
                                gfx::PointF(touch.GetRawX(), touch.GetRawY()),
                                touch.GetUniqueEventId());
}

GestureEventDataPacket GestureEventDataPacket::FromTouchTimeout(
    const GestureEventData& gesture) {
  GestureEventDataPacket packet(gesture.time, TOUCH_TIMEOUT,
                                gfx::PointF(gesture.x, gesture.y),
                                gfx::PointF(gesture.raw_x, gesture.raw_y),
                                gesture.unique_touch_event_id);
  packet.Push(gesture);
  return packet;
}

void GestureEventDataPacket::Ack(bool event_consumed,
                                 bool is_source_touch_event_set_non_blocking) {
  CR_DCHECK(static_cast<int>(ack_state_) == static_cast<int>(AckState::PENDING));
  ack_state_ = event_consumed ? AckState::CONSUMED : AckState::UNCONSUMED;
  for (auto& gesture : gestures_.container()) {
    gesture.details.set_is_source_touch_event_set_non_blocking(
        is_source_touch_event_set_non_blocking);
  }
}

}  // namespace crui
