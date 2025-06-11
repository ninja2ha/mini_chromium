// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/base/ui_base_types.h"

#include "crui/events/event.h"

namespace crui {

MenuSourceType GetMenuSourceTypeForEvent(const crui::Event& event) {
  crui::MenuSourceType source_type = crui::MENU_SOURCE_MOUSE;
  if (event.IsKeyEvent())
    source_type = crui::MENU_SOURCE_KEYBOARD;
  if (event.IsTouchEvent() || event.IsGestureEvent())
    source_type = crui::MENU_SOURCE_TOUCH;
  return source_type;
}

}  // namespace crui
