// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_GESTURE_DETECTION_UI_MOTION_EVENT_H_
#define UI_EVENTS_GESTURE_DETECTION_UI_MOTION_EVENT_H_

#include <stddef.h>

#include <map>
#include <memory>

#include "crbase/time/time.h"
#include "crui/events/event.h"
#include "crui/events/gesture_detection/motion_event_generic.h"
#include "crui/base/ui_export.h"

namespace crui {

// Implementation of MotionEvent which takes a stream of ui::TouchEvents.
class CRUI_EXPORT MotionEventAura : public MotionEventGeneric {
 public:
  MotionEventAura(const MotionEventAura&) = delete;
  MotionEventAura& operator=(const MotionEventAura&) = delete;

  MotionEventAura();
  ~MotionEventAura() override;

  // MotionEventGeneric:
  int GetSourceDeviceId(size_t pointer_index) const override;

  // Returns true iff the touch was valid.
  bool OnTouch(const TouchEvent& touch);

  // We can't cleanup removed touch points immediately upon receipt of a
  // TouchCancel or TouchRelease, as the MotionEvent needs to be able to report
  // information about those touch events. Once the MotionEvent has been
  // processed, we call CleanupRemovedTouchPoints to do the required
  // book-keeping.
  void CleanupRemovedTouchPoints(const TouchEvent& event);

 private:
  bool AddTouch(const TouchEvent& touch);
  void UpdateTouch(const TouchEvent& touch);
  void UpdateCachedAction(const TouchEvent& touch);
  int GetIndexFromId(int id) const;
};

}  // namespace CRui

#endif  // UI_EVENTS_GESTURE_DETECTION_UI_MOTION_EVENT_H_
