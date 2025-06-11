// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/platform_window/platform_window_handler/wm_move_resize_handler.h"

#include "crui/base/class_property.h"
#include "crui/aura/platform_window/platform_window.h"

DEFINE_UI_CLASS_PROPERTY_TYPE(WmMoveResizeHandler*)

namespace crui {

DEFINE_UI_CLASS_PROPERTY_KEY(WmMoveResizeHandler*,
                             kWmMoveResizeHandlerKey,
                             nullptr)

void SetWmMoveResizeHandler(PlatformWindow* platform_window,
                            WmMoveResizeHandler* move_resize_handler) {
  platform_window->SetProperty(kWmMoveResizeHandlerKey, move_resize_handler);
}

WmMoveResizeHandler* GetWmMoveResizeHandler(
    const PlatformWindow& platform_window) {
  return platform_window.GetProperty(kWmMoveResizeHandlerKey);
}

}  // namespace crui
