// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_GESTURE_DETECTION_UI_GESTURE_PROVIDER_H_
#define UI_EVENTS_GESTURE_DETECTION_UI_GESTURE_PROVIDER_H_

#include <stdint.h>

#include <memory>
#include <vector>

#include "crui/events/event.h"
#include "crui/events/gesture_detection/filtered_gesture_provider.h"
#include "crui/events/gesture_detection/gesture_event_data_packet.h"
#include "crui/events/gesture_detection/touch_disposition_gesture_filter.h"
#include "crui/events/gestures/motion_event_aura.h"
#include "crui/base/ui_export.h"

namespace crui {

class GestureProviderAura;

class CRUI_EXPORT GestureProviderAuraClient {
 public:
  virtual ~GestureProviderAuraClient() {}
  virtual void OnGestureEvent(GestureConsumer* consumer,
                              GestureEvent* event) = 0;
};

// Provides gesture detection and dispatch given a sequence of touch events
// and touch event acks.
class CRUI_EXPORT GestureProviderAura : public GestureProviderClient {
 public:
  GestureProviderAura(const GestureProviderAura&) = delete;
  GestureProviderAura& operator=(const GestureProviderAura&) = delete;

  GestureProviderAura(GestureConsumer* consumer,
                      GestureProviderAuraClient* client);
  ~GestureProviderAura() override;

  void set_gesture_consumer(GestureConsumer* consumer) {
    gesture_consumer_ = consumer;
  }

  bool OnTouchEvent(TouchEvent* event);
  void OnTouchEventAck(uint32_t unique_touch_event_id,
                       bool event_consumed,
                       bool is_source_touch_event_set_non_blocking);
  const MotionEventAura& pointer_state() { return pointer_state_; }
  std::vector<std::unique_ptr<GestureEvent>> GetAndResetPendingGestures();
  void OnTouchEnter(int pointer_id, float x, float y);

  void ResetGestureHandlingState();

  // GestureProviderClient implementation
  void OnGestureEvent(const GestureEventData& gesture) override;
  bool RequiresDoubleTapGestureEvents() const override;

 private:
  GestureProviderAuraClient* client_;
  MotionEventAura pointer_state_;
  FilteredGestureProvider filtered_gesture_provider_;

  bool handling_event_;
  std::vector<std::unique_ptr<GestureEvent>> pending_gestures_;

  // |gesture_consumer_| must outlive this object.
  GestureConsumer* gesture_consumer_;
};

}  // namespace crui

#endif  // UI_EVENTS_GESTURE_DETECTION_UI_GESTURE_PROVIDER_H_
