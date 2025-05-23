// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/win/pen_event_processor.h"

#include "crbase/time/time.h"
#include "crui/events/event_utils.h"

namespace crui {
namespace views {
namespace {

int GetFlagsFromPointerMessage(UINT message, const POINTER_INFO& pointer_info) {
  int flags = crui::EF_NONE;
  if (pointer_info.pointerFlags & POINTER_FLAG_FIRSTBUTTON)
    flags |= crui::EF_LEFT_MOUSE_BUTTON;

  if (pointer_info.pointerFlags & POINTER_FLAG_SECONDBUTTON)
    flags |= crui::EF_RIGHT_MOUSE_BUTTON;

  return flags;
}

}  // namespace

PenEventProcessor::PenEventProcessor(crui::SequentialIDGenerator* id_generator,
                                     bool direct_manipulation_enabled)
    : id_generator_(id_generator),
      direct_manipulation_enabled_(direct_manipulation_enabled) {}

PenEventProcessor::~PenEventProcessor() = default;

std::unique_ptr<crui::Event> PenEventProcessor::GenerateEvent(
    UINT message,
    UINT32 pointer_id,
    const POINTER_PEN_INFO& pointer_pen_info,
    const gfx::Point& point) {
  unsigned int mapped_pointer_id = id_generator_->GetGeneratedID(pointer_id);

  // We are now creating a fake mouse event with pointer type of pen from
  // the WM_POINTER message and then setting up an associated pointer
  // details in the MouseEvent which contains the pen's information.
  crui::EventPointerType input_type = crui::EventPointerType::POINTER_TYPE_PEN;
  // For the pointerup event, the penFlags is not set to PEN_FLAG_ERASER, so we
  // have to check if previously the pointer type is an eraser.
  if (pointer_pen_info.penFlags & PEN_FLAG_ERASER) {
    input_type = crui::EventPointerType::POINTER_TYPE_ERASER;
    CR_DCHECK(!eraser_pointer_id_ || *eraser_pointer_id_ == mapped_pointer_id);
    eraser_pointer_id_ = mapped_pointer_id;
  } else if (eraser_pointer_id_ && *eraser_pointer_id_ == mapped_pointer_id &&
             (message == WM_POINTERUP || message == WM_NCPOINTERUP)) {
    input_type = crui::EventPointerType::POINTER_TYPE_ERASER;
    eraser_pointer_id_.reset();
  }

  // convert pressure into a float [0, 1]. The range of the pressure is
  // [0, 1024] as specified on MSDN.
  float pressure = static_cast<float>(pointer_pen_info.pressure) / 1024;
  int rotation_angle = static_cast<int>(pointer_pen_info.rotation) % 180;
  if (rotation_angle < 0)
    rotation_angle += 180;
  int tilt_x = pointer_pen_info.tiltX;
  int tilt_y = pointer_pen_info.tiltY;
  crui::PointerDetails pointer_details(
      input_type, mapped_pointer_id, /* radius_x */ 0.0f, /* radius_y */ 0.0f,
      pressure, 
      static_cast<float>(rotation_angle), 
      static_cast<float>(tilt_x), 
      static_cast<float>(tilt_y), /* tangential_pressure */ 0.0f);

  // If the flag is disabled, we send mouse events for all pen inputs.
  if (!direct_manipulation_enabled_) {
    return GenerateMouseEvent(message, pointer_id, pointer_pen_info.pointerInfo,
                              point, pointer_details);
  }
  bool is_pointer_event =
      message == WM_POINTERENTER || message == WM_POINTERLEAVE;

  // Send MouseEvents when the pen is hovering or any buttons (other than the
  // tip) are depressed when the stylus makes contact with the digitizer. Ensure
  // we read |send_touch_for_pen_| before we process the event as we want to
  // ensure a TouchRelease is sent appropriately at the end when the stylus is
  // no longer in contact with the digitizer.
  bool send_touch = send_touch_for_pen_;
  if (pointer_pen_info.pointerInfo.pointerFlags & POINTER_FLAG_INCONTACT) {
    if (!pen_in_contact_) {
      send_touch = send_touch_for_pen_ =
          (pointer_pen_info.pointerInfo.pointerFlags &
           (POINTER_FLAG_SECONDBUTTON | POINTER_FLAG_THIRDBUTTON |
            POINTER_FLAG_FOURTHBUTTON | POINTER_FLAG_FIFTHBUTTON)) == 0;
    }
    pen_in_contact_ = true;
  } else {
    pen_in_contact_ = false;
    send_touch_for_pen_ = false;
  }

  if (is_pointer_event || !send_touch) {
    return GenerateMouseEvent(message, pointer_id, pointer_pen_info.pointerInfo,
                              point, pointer_details);
  }

  return GenerateTouchEvent(message, pointer_id, pointer_pen_info.pointerInfo,
                            point, pointer_details);
}

std::unique_ptr<crui::Event> PenEventProcessor::GenerateMouseEvent(
    UINT message,
    UINT32 pointer_id,
    const POINTER_INFO& pointer_info,
    const gfx::Point& point,
    const crui::PointerDetails& pointer_details) {
  crui::EventType event_type = crui::ET_MOUSE_MOVED;
  int flag = GetFlagsFromPointerMessage(message, pointer_info);
  int changed_flag = crui::EF_NONE;
  int click_count = 0;
  switch (message) {
    case WM_POINTERDOWN:
    case WM_NCPOINTERDOWN:
      event_type = crui::ET_MOUSE_PRESSED;
      if (pointer_info.ButtonChangeType == POINTER_CHANGE_FIRSTBUTTON_DOWN)
        changed_flag = crui::EF_LEFT_MOUSE_BUTTON;
      else
        changed_flag = crui::EF_RIGHT_MOUSE_BUTTON;
      click_count = 1;
      sent_mouse_down_[pointer_id] = true;
      break;
    case WM_POINTERUP:
    case WM_NCPOINTERUP:
      event_type = crui::ET_MOUSE_RELEASED;
      if (pointer_info.ButtonChangeType == POINTER_CHANGE_FIRSTBUTTON_UP) {
        flag |= crui::EF_LEFT_MOUSE_BUTTON;
        changed_flag = crui::EF_LEFT_MOUSE_BUTTON;
      } else {
        flag |= crui::EF_RIGHT_MOUSE_BUTTON;
        changed_flag = crui::EF_RIGHT_MOUSE_BUTTON;
      }
      id_generator_->ReleaseNumber(pointer_id);
      click_count = 1;
      if (sent_mouse_down_.count(pointer_id) == 0 ||
          !sent_mouse_down_[pointer_id])
        return nullptr;
      sent_mouse_down_[pointer_id] = false;
      break;
    case WM_POINTERUPDATE:
    case WM_NCPOINTERUPDATE:
      event_type = crui::ET_MOUSE_DRAGGED;
      if (flag == crui::EF_NONE)
        event_type = crui::ET_MOUSE_MOVED;
      break;
    case WM_POINTERENTER:
      event_type = crui::ET_MOUSE_ENTERED;
      break;
    case WM_POINTERLEAVE:
      event_type = crui::ET_MOUSE_EXITED;
      id_generator_->ReleaseNumber(pointer_id);
      break;
    default:
      CR_NOTREACHED();
  }
  std::unique_ptr<crui::Event> event = std::make_unique<crui::MouseEvent>(
      event_type, point, point, crui::EventTimeForNow(),
      flag | crui::GetModifiersFromKeyState(), changed_flag, pointer_details);
  event->AsMouseEvent()->SetClickCount(click_count);
  return event;
}

std::unique_ptr<crui::Event> PenEventProcessor::GenerateTouchEvent(
    UINT message,
    UINT32 pointer_id,
    const POINTER_INFO& pointer_info,
    const gfx::Point& point,
    const crui::PointerDetails& pointer_details) {
  int flags = GetFlagsFromPointerMessage(message, pointer_info);

  crui::EventType event_type = crui::ET_TOUCH_MOVED;
  switch (message) {
    case WM_POINTERDOWN:
    case WM_NCPOINTERDOWN:
      event_type = crui::ET_TOUCH_PRESSED;
      sent_touch_start_[pointer_id] = true;
      break;
    case WM_POINTERUP:
    case WM_NCPOINTERUP:
      event_type = crui::ET_TOUCH_RELEASED;
      id_generator_->ReleaseNumber(pointer_id);
      if (sent_touch_start_.count(pointer_id) == 0 ||
          !sent_touch_start_[pointer_id])
        return nullptr;
      sent_touch_start_[pointer_id] = false;
      break;
    case WM_POINTERUPDATE:
    case WM_NCPOINTERUPDATE:
      event_type = crui::ET_TOUCH_MOVED;
      break;
    default:
      CR_NOTREACHED();
  }

  const cr::TimeTicks event_time = crui::EventTimeForNow();

  std::unique_ptr<crui::TouchEvent> event = std::make_unique<crui::TouchEvent>(
      event_type, point, event_time, pointer_details,
      flags | crui::GetModifiersFromKeyState());
  event->set_hovering(event_type == crui::ET_TOUCH_RELEASED);
  ///event->latency()->AddLatencyNumberWithTimestamp(
  ///    ui::INPUT_EVENT_LATENCY_ORIGINAL_COMPONENT, event_time);
  return std::move(event);
}

}  // namespace views
}  // namespace crui
