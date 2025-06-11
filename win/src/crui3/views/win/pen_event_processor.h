// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_WIN_PEN_EVENT_PROCESSOR_H_
#define UI_VIEWS_WIN_PEN_EVENT_PROCESSOR_H_

#include <windows.h>

#include <memory>

#include "crbase/containers/optional.h"
#include "crui/events/event.h"
#include "crui/gfx/geometry/point.h"
#include "crui/gfx/sequential_id_generator.h"
#include "crui/base/ui_export.h"

namespace crui {
namespace views {

// This class handles the processing pen event state information
// from native Windows events and returning appropriate
// ui::Events for the current state.
class CRUI_EXPORT PenEventProcessor {
 public:
  PenEventProcessor(const PenEventProcessor&) = delete;
  PenEventProcessor& operator=(const PenEventProcessor&) = delete;

  // |id_generator| must outlive this object's lifecycle.
  PenEventProcessor(crui::SequentialIDGenerator* id_generator,
                    bool direct_manipulation_enabled);
  ~PenEventProcessor();

  // Generate an appropriate ui::Event for a given pen pointer.
  // May return nullptr if no event should be dispatched.
  std::unique_ptr<crui::Event> GenerateEvent(
      UINT message,
      UINT32 pointer_id,
      const POINTER_PEN_INFO& pen_pointer_info,
      const gfx::Point& point);

 private:
  std::unique_ptr<crui::Event> GenerateMouseEvent(
      UINT message,
      UINT32 pointer_id,
      const POINTER_INFO& pointer_info,
      const gfx::Point& point,
      const crui::PointerDetails& pointer_details);
  std::unique_ptr<crui::Event> GenerateTouchEvent(
      UINT message,
      UINT32 pointer_id,
      const POINTER_INFO& pointer_info,
      const gfx::Point& point,
      const crui::PointerDetails& pointer_details);

  crui::SequentialIDGenerator* id_generator_;
  bool direct_manipulation_enabled_;
  bool pen_in_contact_ = false;
  bool send_touch_for_pen_ = false;

  // There may be more than one pen used at the same time.
  cr::flat_map<UINT32, bool> sent_mouse_down_;
  cr::flat_map<UINT32, bool> sent_touch_start_;

  cr::Optional<unsigned int> eraser_pointer_id_;
};

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_WIN_PEN_EVENT_PROCESSOR_H_
