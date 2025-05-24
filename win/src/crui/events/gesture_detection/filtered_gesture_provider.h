// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_GESTURE_DETECTION_FILTERED_GESTURE_PROVIDER_H_
#define UI_EVENTS_GESTURE_DETECTION_FILTERED_GESTURE_PROVIDER_H_

#include <stdint.h>
#include <memory>

#include "crbase/compiler_specific.h"
#include "crui/events/gesture_detection/gesture_event_data_packet.h"
#include "crui/events/gesture_detection/gesture_provider.h"
#include "crui/events/gesture_detection/touch_disposition_gesture_filter.h"

namespace crui {

// Provides filtered gesture detection and dispatch given a sequence of touch
// events and touch event acks.
class CRUI_EXPORT FilteredGestureProvider
    : public crui::TouchDispositionGestureFilterClient,
      public crui::GestureProviderClient {
 public:
  FilteredGestureProvider(const FilteredGestureProvider&) = delete;
  FilteredGestureProvider& operator=(const FilteredGestureProvider&) = delete;

  // |client| will be offered all gestures detected by the |gesture_provider_|
  // and allowed by the |gesture_filter_|.
  FilteredGestureProvider(const GestureProvider::Config& config,
                          GestureProviderClient* client);
  ~FilteredGestureProvider() final;

  void UpdateConfig(const GestureProvider::Config& config);

  struct TouchHandlingResult {
    TouchHandlingResult();

    // True if |event| was both valid and successfully handled by the
    // gesture provider. Otherwise, false, in which case the caller should drop
    // |event| and cease further propagation.
    bool succeeded;

    // Whether |event| occurred beyond the touch slop region.
    bool moved_beyond_slop_region;
  };
  TouchHandlingResult OnTouchEvent(const MotionEvent& event) 
      CR_WARN_UNUSED_RESULT;

  // To be called upon asynchronous and synchronous ack of an event that was
  // forwarded after a successful call to |OnTouchEvent()|.
  void OnTouchEventAck(uint32_t unique_event_id,
                       bool event_consumed,
                       bool is_source_touch_event_set_non_blocking);

  void ResetGestureHandlingState();

  // Methods delegated to |gesture_provider_|.
  void ResetDetection();
  void SetMultiTouchZoomSupportEnabled(bool enabled);
  void SetDoubleTapSupportForPlatformEnabled(bool enabled);
  void SetDoubleTapSupportForPageEnabled(bool enabled);
  const crui::MotionEvent* GetCurrentDownEvent() const;

 private:
  // GestureProviderClient implementation.
  void OnGestureEvent(const crui::GestureEventData& event) override;
  bool RequiresDoubleTapGestureEvents() const override;

  // TouchDispositionGestureFilterClient implementation.
  void ForwardGestureEvent(const crui::GestureEventData& event) override;

  GestureProviderClient* const client_;

  std::unique_ptr<crui::GestureProvider> gesture_provider_;
  crui::TouchDispositionGestureFilter gesture_filter_;

  bool handling_event_;
  bool any_touch_moved_beyond_slop_region_;
  crui::GestureEventDataPacket pending_gesture_packet_;
};

}  // namespace crui

#endif  // UI_EVENTS_GESTURE_DETECTION_FILTERED_GESTURE_PROVIDER_H_
