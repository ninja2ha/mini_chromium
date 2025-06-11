// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/events/event_utils.h"
#include "crui/events/win/events_win_utils.h"

namespace crui {

EventType EventTypeFromNative(const PlatformEvent& native_event) {
  return EventTypeFromMSG(native_event);
}

int EventFlagsFromNative(const PlatformEvent& native_event) {
  return EventFlagsFromMSG(native_event);
}

cr::TimeTicks EventTimeFromNative(const PlatformEvent& native_event) {
  return EventTimeFromMSG(native_event);
}

gfx::PointF EventLocationFromNative(const PlatformEvent& native_event) {
  return gfx::PointF(EventLocationFromMSG(native_event));
}

gfx::Point EventSystemLocationFromNative(const PlatformEvent& native_event) {
  return EventSystemLocationFromMSG(native_event);
}

KeyboardCode KeyboardCodeFromNative(const PlatformEvent& native_event) {
  return KeyboardCodeFromMSG(native_event);
}

DomCode CodeFromNative(const PlatformEvent& native_event) {
  return CodeFromMSG(native_event);
}

bool IsCharFromNative(const PlatformEvent& native_event) {
  return IsCharFromMSG(native_event);
}

int GetChangedMouseButtonFlagsFromNative(const PlatformEvent& native_event) {
  return GetChangedMouseButtonFlagsFromMSG(native_event);
}

PointerDetails GetMousePointerDetailsFromNative(
    const PlatformEvent& native_event) {
  return GetMousePointerDetailsFromMSG(native_event);
}

gfx::Vector2d GetMouseWheelOffset(const PlatformEvent& native_event) {
  return GetMouseWheelOffsetFromMSG(native_event);
}

MSG CopyNativeEvent(const PlatformEvent& event) {
  return CopyMSGEvent(event);
}

void ReleaseCopiedNativeEvent(const PlatformEvent& event) {}

PointerDetails GetTouchPointerDetailsFromNative(const MSG& native_event) {
  CR_NOTIMPLEMENTED();
  return PointerDetails(EventPointerType::POINTER_TYPE_TOUCH,
                        /* pointer_id*/ 0,
                        /* radius_x */ 1.0,
                        /* radius_y */ 1.0,
                        /* force */ 0.f);
}

bool GetScrollOffsets(const PlatformEvent& native_event,
                      float* x_offset,
                      float* y_offset,
                      float* x_offset_ordinal,
                      float* y_offset_ordinal,
                      int* finger_count,
                      EventMomentumPhase* momentum_phase) {
  return GetScrollOffsetsFromMSG(native_event);
}

bool GetFlingData(const PlatformEvent& native_event,
                  float* vx,
                  float* vy,
                  float* vx_ordinal,
                  float* vy_ordinal,
                  bool* is_cancel) {
  // Not supported in Windows.
  CR_NOTIMPLEMENTED();
  return false;
}

}  // namespace crui
