// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_GESTURES_GESTURE_RECOGNIZER_IMPL_H_
#define UI_EVENTS_GESTURES_GESTURE_RECOGNIZER_IMPL_H_

#include <stdint.h>

#include <map>
#include <memory>
#include <vector>

#include "crui/events/event_constants.h"
#include "crui/events/gestures/gesture_provider_aura.h"
#include "crui/events/gestures/gesture_recognizer.h"
#include "crui/gfx/geometry/point.h"
#include "crui/base/ui_export.h"

namespace crui {

class GestureConsumer;
class GestureEvent;
class GestureEventHelper;
class TouchEvent;

// TODO(tdresser): Once the unified gesture recognition process sticks
// (crbug.com/332418), GestureRecognizerImpl can be cleaned up
// significantly.
class CRUI_EXPORT GestureRecognizerImpl : public GestureRecognizer,
                                          public GestureProviderAuraClient {
 public:
  typedef std::map<int, GestureConsumer*> TouchIdToConsumerMap;

  GestureRecognizerImpl(const GestureRecognizerImpl&) = delete;
  GestureRecognizerImpl& operator=(const GestureRecognizerImpl&) = delete;

  GestureRecognizerImpl();
  ~GestureRecognizerImpl() override;

  std::vector<GestureEventHelper*>& helpers() { return helpers_; }

  // Returns a list of events of type |type|, one for each pointer down on
  // |consumer|. Event locations are pulled from the active pointers.
  std::vector<std::unique_ptr<TouchEvent>> GetEventPerPointForConsumer(
      GestureConsumer* consumer,
      EventType type);

  // Overridden from GestureRecognizer
  GestureConsumer* GetTouchLockedTarget(const TouchEvent& event) override;
  GestureConsumer* GetTargetForLocation(const gfx::PointF& location,
                                        int source_device_id) override;
  void CancelActiveTouchesExcept(GestureConsumer* not_cancelled) override;
  void CancelActiveTouchesOn(
      const std::vector<GestureConsumer*>& consumers) override;
  void TransferEventsTo(
      GestureConsumer* current_consumer,
      GestureConsumer* new_consumer,
      TransferTouchesBehavior transfer_touches_behavior) override;
  std::vector<std::unique_ptr<crui::TouchEvent>> ExtractTouches(
      GestureConsumer* consumer) override;
  void TransferTouches(GestureConsumer* consumer,
                       const std::vector<std::unique_ptr<crui::TouchEvent>>&
                           touch_events) override;
  bool GetLastTouchPointForTarget(GestureConsumer* consumer,
                                  gfx::PointF* point) override;
  bool CancelActiveTouches(GestureConsumer* consumer) override;

 protected:
  virtual GestureProviderAura* GetGestureProviderForConsumer(
      GestureConsumer* c);

  // Overridden from GestureRecognizer
  bool ProcessTouchEventPreDispatch(TouchEvent* event,
                                    GestureConsumer* consumer) override;

 private:
  // Sets up the target consumer for gestures based on the touch-event.
  void SetupTargets(const TouchEvent& event, GestureConsumer* consumer);

  void DispatchGestureEvent(GestureConsumer* raw_input_consumer,
                            GestureEvent* event);

  Gestures AckTouchEvent(uint32_t unique_event_id,
                         crui::EventResult result,
                         bool is_source_touch_event_set_non_blocking,
                         GestureConsumer* consumer) override;

  void CancelActiveTouchesExceptImpl(GestureConsumer* not_cancelled);
  bool CancelActiveTouchesImpl(GestureConsumer* consumer);

  bool CleanupStateForConsumer(GestureConsumer* consumer) override;
  void AddGestureEventHelper(GestureEventHelper* helper) override;
  void RemoveGestureEventHelper(GestureEventHelper* helper) override;

  // Overridden from GestureProviderAuraClient
  void OnGestureEvent(GestureConsumer* raw_input_consumer,
                      GestureEvent* event) override;

  // Convenience method to find the GestureEventHelper that can dispatch events
  // to a specific |consumer|.
  GestureEventHelper* FindDispatchHelperForConsumer(GestureConsumer* consumer);
  std::map<GestureConsumer*, std::unique_ptr<GestureProviderAura>>
      consumer_gesture_provider_;

  // Maps an event via its |unique_event_id| to the corresponding gesture
  // provider. This avoids any invalid reference while routing ACKs for events
  // that may arise post |TransferEventsTo()| function call.
  // See http://crbug.com/698843 for more info.
  std::map<uint32_t, GestureProviderAura*> event_to_gesture_provider_;

  // |touch_id_target_| maps a touch-id to its target window.
  // touch-ids are removed from |touch_id_target_| on
  // ET_TOUCH_RELEASE and ET_TOUCH_CANCEL.
  TouchIdToConsumerMap touch_id_target_;

  std::vector<GestureEventHelper*> helpers_;
};

}  // namespace crui

#endif  // UI_EVENTS_GESTURES_GESTURE_RECOGNIZER_IMPL_H_
