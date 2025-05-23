// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/env_input_state_controller.h"

#include "crui/aura/client/screen_position_client.h"
#include "crui/aura/env.h"
#include "crui/events/event.h"
#include "crui/gfx/geometry/point.h"

namespace crui {
namespace aura {

EnvInputStateController::EnvInputStateController(Env* env) : env_(env) {}

EnvInputStateController::~EnvInputStateController() = default;

void EnvInputStateController::UpdateStateForMouseEvent(
    const Window* window,
    const crui::MouseEvent& event) {
  switch (event.type()) {
    case crui::ET_MOUSE_PRESSED:
      env_->set_mouse_button_flags(event.button_flags());
      break;
    case crui::ET_MOUSE_RELEASED:
      env_->set_mouse_button_flags(event.button_flags() &
                                   ~event.changed_button_flags());
      break;
    default:
      break;
  }

  // If a synthesized event is created from a native event (e.g. EnterNotify
  // XEvents), then we should take the location as we would for a
  // non-synthesized event.
  if (event.type() != crui::ET_MOUSE_CAPTURE_CHANGED &&
      (!(event.flags() & crui::EF_IS_SYNTHESIZED) || event.HasNativeEvent())) {
    SetLastMouseLocation(window, event.root_location());
  }
}

void EnvInputStateController::UpdateStateForTouchEvent(
    const crui::TouchEvent& event) {
  switch (event.type()) {
    case crui::ET_TOUCH_PRESSED:
      touch_ids_down_ |= (1 << event.pointer_details().id);
      env_->set_touch_down(touch_ids_down_ != 0);
      break;

    // Handle ET_TOUCH_CANCELLED only if it has a native event.
    case crui::ET_TOUCH_CANCELLED:
      if (!event.HasNativeEvent())
        break;
      CR_FALLTHROUGH;
    case crui::ET_TOUCH_RELEASED:
      touch_ids_down_ = (touch_ids_down_ | (1 << event.pointer_details().id)) ^
                        (1 << event.pointer_details().id);
      env_->set_touch_down(touch_ids_down_ != 0);
      break;

    case crui::ET_TOUCH_MOVED:
      break;

    default:
      CR_NOTREACHED();
      break;
  }
}

void EnvInputStateController::SetLastMouseLocation(
    const Window* root_window,
    const gfx::Point& location_in_root) const {
  // If |root_window| is null, we are only using the event to update event
  // states, so we shouldn't update mouse location.
  if (!root_window)
    return;

  client::ScreenPositionClient* client =
      client::GetScreenPositionClient(root_window);
  if (client) {
    gfx::Point location_in_screen = location_in_root;
    client->ConvertPointToScreen(root_window, &location_in_screen);
    env_->SetLastMouseLocation(location_in_screen);
  } else {
    env_->SetLastMouseLocation(location_in_root);
  }
}

}  // namespace aura
}  // namespace crui
