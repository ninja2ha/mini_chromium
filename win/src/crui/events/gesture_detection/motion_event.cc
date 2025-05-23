// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/events/gesture_detection/motion_event.h"

#include "crbase/logging.h"
#include "crui/events/gesture_detection/motion_event_generic.h"

namespace crui {

size_t MotionEvent::GetHistorySize() const {
  return 0;
}

cr::TimeTicks MotionEvent::GetHistoricalEventTime(
    size_t historical_index) const {
  CR_NOTIMPLEMENTED();
  return cr::TimeTicks();
}

float MotionEvent::GetHistoricalTouchMajor(size_t pointer_index,
                                           size_t historical_index) const {
  CR_NOTIMPLEMENTED();
  return 0.f;
}

float MotionEvent::GetHistoricalX(size_t pointer_index,
                                  size_t historical_index) const {
  CR_NOTIMPLEMENTED();
  return 0.f;
}

float MotionEvent::GetHistoricalY(size_t pointer_index,
                                  size_t historical_index) const {
  CR_NOTIMPLEMENTED();
  return 0.f;
}

int MotionEvent::FindPointerIndexOfId(int id) const {
  const size_t pointer_count = GetPointerCount();
  for (size_t i = 0; i < pointer_count; ++i) {
    if (GetPointerId(i) == id)
      return static_cast<int>(i);
  }
  return -1;
}

int MotionEvent::GetSourceDeviceId(size_t pointer_index) const {
  CR_NOTIMPLEMENTED();
  return 0;
}

std::unique_ptr<MotionEvent> MotionEvent::Clone() const {
  return MotionEventGeneric::CloneEvent(*this);
}

std::unique_ptr<MotionEvent> MotionEvent::Cancel() const {
  return MotionEventGeneric::CancelEvent(*this);
}

std::ostream& operator<<(std::ostream& stream,
                         const MotionEvent::Action action) {
  return stream << static_cast<int>(action);
}
std::ostream& operator<<(std::ostream& stream,
                         const MotionEvent::ToolType tool_type) {
  return stream << static_cast<int>(tool_type);
}

}  // namespace crui
