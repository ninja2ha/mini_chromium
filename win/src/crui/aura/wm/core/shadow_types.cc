// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/wm//core/shadow_types.h"

#include "crui/base/class_property.h"

namespace crui {
namespace wm {

DEFINE_UI_CLASS_PROPERTY_KEY(int, kShadowElevationKey, kShadowElevationDefault)

void SetShadowElevation(aura::Window* window, int elevation) {
  window->SetProperty(kShadowElevationKey, elevation);
}

int GetDefaultShadowElevationForWindow(const aura::Window* window) {
  switch (window->type()) {
    case aura::client::WINDOW_TYPE_NORMAL:
      return kShadowElevationInactiveWindow;

    case aura::client::WINDOW_TYPE_MENU:
    case aura::client::WINDOW_TYPE_TOOLTIP:
      return kShadowElevationMenuOrTooltip;

    default:
      return kShadowElevationNone;
  }
}

int GetShadowElevationConvertDefault(const aura::Window* window) {
  int elevation = window->GetProperty(kShadowElevationKey);
  return elevation == kShadowElevationDefault
             ? GetDefaultShadowElevationForWindow(window)
             : elevation;
}

}  // namespace wm
}  // namespace crui
