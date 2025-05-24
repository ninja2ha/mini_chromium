// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_SCREEN_H_
#define UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_SCREEN_H_

#include "crui/base/ui_export.h"

namespace crui {

namespace display {
class Screen;
}  // namespace display

namespace views {

// Creates a Screen that represents the screen of the environment that hosts
// a WindowTreeHost. Caller owns the result.
CRUI_EXPORT display::Screen* CreateDesktopScreen();

CRUI_EXPORT void InstallDesktopScreenIfNecessary();

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_SCREEN_H_
