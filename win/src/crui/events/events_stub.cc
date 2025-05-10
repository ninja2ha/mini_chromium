// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include "crbase/logging.h"
#include "crbase/time/time.h"
#include "crui/events/event_utils.h"
#include "crui/events/keycodes/dom/dom_code.h"
#include "crui/gfx/geometry/point.h"
#include "crui/gfx/geometry/vector2d.h"
#include "crui/base/build_platform.h"

namespace crui {

// Stub implementations of platform-specific methods in events_util.h, built
// on platforms that currently do not have a complete implementation of events.

EventType EventTypeFromNative(const PlatformEvent& native_event) {
  CR_NOTIMPLEMENTED();
  return ET_UNKNOWN;
}

int EventFlagsFromNative(const PlatformEvent& native_event) {
  CR_NOTIMPLEMENTED();
  return 0;
}

cr::TimeTicks EventTimeFromNative(const PlatformEvent& native_event) {
  CR_NOTIMPLEMENTED();
  return cr::TimeTicks();
}

gfx::PointF EventLocationFromNative(const PlatformEvent& native_event) {
  CR_NOTIMPLEMENTED();
  return gfx::PointF();
}

gfx::Point EventSystemLocationFromNative(const PlatformEvent& native_event) {
  CR_NOTIMPLEMENTED();
  return gfx::Point();
}

int EventButtonFromNative(const PlatformEvent& native_event) {
  CR_NOTIMPLEMENTED();
  return 0;
}

int GetChangedMouseButtonFlagsFromNative(const PlatformEvent& native_event) {
  CR_NOTIMPLEMENTED();
  return 0;
}

PointerDetails GetMousePointerDetailsFromNative(
    const PlatformEvent& native_event) {
  return PointerDetails(EventPointerType::POINTER_TYPE_MOUSE);
}

gfx::Vector2d GetMouseWheelOffset(const PlatformEvent& native_event) {
  CR_NOTIMPLEMENTED();
  return gfx::Vector2d();
}

PlatformEvent CopyNativeEvent(const PlatformEvent& event) {
  CR_NOTIMPLEMENTED() 
      << "Don't know how to copy PlatformEvent for this platform";
#if defined(MINI_CHROMIUM_OS_WIN)
  return PlatformEvent();
#else
  return NULL;
#endif
}

void ReleaseCopiedNativeEvent(const PlatformEvent& event) {}

PointerDetails GetTouchPointerDetailsFromNative(
    const PlatformEvent& native_event) {
  CR_NOTIMPLEMENTED();
  return PointerDetails(EventPointerType::POINTER_TYPE_UNKNOWN,
                        /* radius_x */ 1.0,
                        /* radius_y */ 1.0,
                        /* force */ 0.f,
                        /* twist */ 0.f,
                        /* tilt_x */ 0.f,
                        /* tilt_y */ 0.f);
}

bool GetScrollOffsets(const PlatformEvent& native_event,
                      float* x_offset,
                      float* y_offset,
                      float* x_offset_ordinal,
                      float* y_offset_ordinal,
                      int* finger_count,
                      EventMomentumPhase* momentum_phase) {
  CR_NOTIMPLEMENTED();
  return false;
}

bool GetFlingData(const PlatformEvent& native_event,
                  float* vx,
                  float* vy,
                  float* vx_ordinal,
                  float* vy_ordinal,
                  bool* is_cancel) {
  CR_NOTIMPLEMENTED();
  return false;
}

KeyboardCode KeyboardCodeFromNative(const PlatformEvent& native_event) {
  CR_NOTIMPLEMENTED();
  return static_cast<KeyboardCode>(0);
}

DomCode CodeFromNative(const PlatformEvent& native_event) {
  CR_NOTIMPLEMENTED();
  return DomCode::NONE;
}

bool IsCharFromNative(const PlatformEvent& native_event) {
  CR_NOTIMPLEMENTED();
  return false;
}

uint32_t WindowsKeycodeFromNative(const PlatformEvent& native_event) {
  CR_NOTIMPLEMENTED();
  return 0;
}

uint16_t TextFromNative(const PlatformEvent& native_event) {
  CR_NOTIMPLEMENTED();
  return 0;
}

uint16_t UnmodifiedTextFromNative(const PlatformEvent& native_event) {
  CR_NOTIMPLEMENTED();
  return 0;
}


}  // namespace ui
