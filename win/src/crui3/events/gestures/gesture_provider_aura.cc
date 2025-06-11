// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/events/gestures/gesture_provider_aura.h"

#include <utility>

#include "crbase/auto_reset.h"
#include "crbase/logging.h"
#include "crui/events/event.h"
#include "crui/events/event_utils.h"
#include "crui/events/gesture_detection/gesture_configuration.h"
#include "crui/events/gesture_detection/gesture_event_data.h"
#include "crui/events/gesture_detection/gesture_provider_config_helper.h"

namespace crui {

namespace {
constexpr bool kDoubleTapPlatformSupport = false;
}  // namespace

GestureProviderAura::GestureProviderAura(GestureConsumer* consumer,
                                         GestureProviderAuraClient* client)
    : client_(client),
      filtered_gesture_provider_(
          GetGestureProviderConfig(GestureProviderConfigType::CURRENT_PLATFORM),
          this),
      handling_event_(false),
      gesture_consumer_(consumer) {
  filtered_gesture_provider_.SetDoubleTapSupportForPlatformEnabled(
      kDoubleTapPlatformSupport);
}

GestureProviderAura::~GestureProviderAura() {}

bool GestureProviderAura::OnTouchEvent(TouchEvent* event) {
  if (!pointer_state_.OnTouch(*event))
    return false;

  auto result = filtered_gesture_provider_.OnTouchEvent(pointer_state_);
  pointer_state_.CleanupRemovedTouchPoints(*event);

  if (!result.succeeded)
    return false;

  event->set_may_cause_scrolling(result.moved_beyond_slop_region);
  return true;
}

void GestureProviderAura::OnTouchEventAck(
    uint32_t unique_touch_event_id,
    bool event_consumed,
    bool is_source_touch_event_set_non_blocking) {
  CR_DCHECK(pending_gestures_.empty());
  CR_DCHECK(!handling_event_);
  cr::AutoReset<bool> handling_event(&handling_event_, true);
  filtered_gesture_provider_.OnTouchEventAck(
      unique_touch_event_id, event_consumed,
      is_source_touch_event_set_non_blocking);
}

void GestureProviderAura::ResetGestureHandlingState() {
  filtered_gesture_provider_.ResetGestureHandlingState();
}

void GestureProviderAura::OnGestureEvent(const GestureEventData& gesture) {
  std::unique_ptr<crui::GestureEvent> event(
      new crui::GestureEvent(gesture.x, gesture.y, gesture.flags,
                             gesture.time, gesture.details,
                             gesture.unique_touch_event_id));

  if (!handling_event_) {
    // Dispatching event caused by timer.
    client_->OnGestureEvent(gesture_consumer_, event.get());
  } else {
    pending_gestures_.push_back(std::move(event));
  }
}

bool GestureProviderAura::RequiresDoubleTapGestureEvents() const {
  return gesture_consumer_->RequiresDoubleTapGestureEvents();
}

std::vector<std::unique_ptr<GestureEvent>>
GestureProviderAura::GetAndResetPendingGestures() {
  std::vector<std::unique_ptr<GestureEvent>> result;
  result.swap(pending_gestures_);
  return result;
}

void GestureProviderAura::OnTouchEnter(int pointer_id, float x, float y) {
  auto touch_event = std::make_unique<TouchEvent>(
      ET_TOUCH_PRESSED, gfx::Point(), crui::EventTimeForNow(),
      PointerDetails(crui::EventPointerType::POINTER_TYPE_TOUCH, pointer_id),
      EF_IS_SYNTHESIZED);
  gfx::PointF point(x, y);
  touch_event->set_location_f(point);
  touch_event->set_root_location_f(point);

  OnTouchEvent(touch_event.get());
  OnTouchEventAck(touch_event->unique_event_id(), true /* event_consumed */,
                  false /* is_source_touch_event_set_non_blocking */);
}

}  // namespace crui
