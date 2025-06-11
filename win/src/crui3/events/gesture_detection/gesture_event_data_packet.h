// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_GESTURE_DETECTION_GESTURE_EVENT_DATA_PACKET_H_
#define UI_EVENTS_GESTURE_DETECTION_GESTURE_EVENT_DATA_PACKET_H_

#include <stddef.h>
#include <stdint.h>

#include "crbase/containers/stack_container.h"
#include "crui/base/ui_export.h"
#include "crui/events/gesture_detection/gesture_event_data.h"

namespace crui {

class MotionEvent;

// Acts as a transport container for gestures created (directly or indirectly)
// by a touch event.
class CRUI_EXPORT GestureEventDataPacket {
 public:
  enum GestureSource {
    UNDEFINED = -1,        // Used only for a default-constructed packet.
    INVALID,               // The source of the gesture was invalid.
    TOUCH_SEQUENCE_START,  // The start of a new gesture sequence.
    TOUCH_SEQUENCE_END,    // The end of a gesture sequence.
    TOUCH_SEQUENCE_CANCEL, // The gesture sequence was cancelled.
    TOUCH_START,           // A touch down occured during a gesture sequence.
    TOUCH_MOVE,            // A touch move occured during a gesture sequence.
    TOUCH_END,             // A touch up occured during a gesture sequence.
    TOUCH_TIMEOUT,         // Timeout from an existing gesture sequence.
  };

  enum class AckState {
    PENDING,
    CONSUMED,
    UNCONSUMED,
  };

  GestureEventDataPacket();
  GestureEventDataPacket(const GestureEventDataPacket& other);
  ~GestureEventDataPacket();
  GestureEventDataPacket& operator=(const GestureEventDataPacket& other);

  // Factory methods for creating a packet from a particular event.
  static GestureEventDataPacket FromTouch(const crui::MotionEvent& touch);
  static GestureEventDataPacket FromTouchTimeout(
      const GestureEventData& gesture);

  // Pushes into the GestureEventDataPacket a copy of |gesture| that has the
  // same unique_touch_event_id as the data packet.
  void Push(const GestureEventData& gesture);

  const cr::TimeTicks& timestamp() const { return timestamp_; }
  const GestureEventData& gesture(size_t i) const { return gestures_[i]; }
  size_t gesture_count() const { return gestures_->size(); }
  GestureSource gesture_source() const { return gesture_source_; }
  const gfx::PointF& touch_location() const { return touch_location_; }
  const gfx::PointF& raw_touch_location() const { return raw_touch_location_; }

  // We store the ack with the packet until the packet reaches the
  // head of the queue, and then we handle the ack.
  void Ack(bool event_consumed, bool is_source_touch_event_set_non_blocking);
  AckState ack_state() { return ack_state_; }
  uint32_t unique_touch_event_id() const { return unique_touch_event_id_; }

 private:
  GestureEventDataPacket(cr::TimeTicks timestamp,
                         GestureSource source,
                         const gfx::PointF& touch_location,
                         const gfx::PointF& raw_touch_location,
                         uint32_t unique_touch_event_id);

  enum { kTypicalMaxGesturesPerTouch = 5 };
  cr::TimeTicks timestamp_;
  cr::StackVector<GestureEventData, kTypicalMaxGesturesPerTouch> gestures_;
  gfx::PointF touch_location_;
  gfx::PointF raw_touch_location_;
  GestureSource gesture_source_;
  AckState ack_state_;
  uint32_t unique_touch_event_id_;
};

}  // namespace crui

#endif  // UI_EVENTS_GESTURE_DETECTION_GESTURE_EVENT_DATA_PACKET_H_
