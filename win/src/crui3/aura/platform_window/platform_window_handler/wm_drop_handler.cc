// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/platform_window/platform_window_handler/wm_drop_handler.h"

#include "crui/base/class_property.h"
#include "crui/aura/platform_window/platform_window.h"

DEFINE_UI_CLASS_PROPERTY_TYPE(crui::WmDropHandler*)

namespace crui {

DEFINE_UI_CLASS_PROPERTY_KEY(WmDropHandler*, kWmDropHandlerKey, nullptr)

void SetWmDropHandler(PlatformWindow* platform_window,
                      WmDropHandler* drop_handler) {
  platform_window->SetProperty(kWmDropHandlerKey, drop_handler);
}

WmDropHandler* GetWmDropHandler(const PlatformWindow& platform_window) {
  return platform_window.GetProperty(kWmDropHandlerKey);
}

}  // namespace crui
