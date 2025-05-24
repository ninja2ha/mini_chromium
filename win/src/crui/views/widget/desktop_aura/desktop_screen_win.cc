// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/widget/desktop_aura/desktop_screen_win.h"

#include "crbase/logging.h"
#include "crui/aura/window.h"
#include "crui/aura/window_event_dispatcher.h"
#include "crui/aura/window_tree_host.h"
#include "crui/display/display.h"
#include "crui/views/widget/desktop_aura/desktop_screen.h"
#include "crui/views/widget/desktop_aura/desktop_window_tree_host_win.h"

namespace crui {
namespace views {

////////////////////////////////////////////////////////////////////////////////
// DesktopScreenWin, public:

DesktopScreenWin::DesktopScreenWin() = default;

DesktopScreenWin::~DesktopScreenWin() = default;

////////////////////////////////////////////////////////////////////////////////
// DesktopScreenWin, display::win::ScreenWin implementation:

display::Display DesktopScreenWin::GetDisplayMatching(
    const gfx::Rect& match_rect) const {
  return GetDisplayNearestPoint(match_rect.CenterPoint());
}

HWND DesktopScreenWin::GetHWNDFromNativeView(gfx::NativeView window) const {
  aura::WindowTreeHost* host = window->GetHost();
  return host ? host->GetAcceleratedWidget() : nullptr;
}

gfx::NativeWindow DesktopScreenWin::GetNativeWindowFromHWND(HWND hwnd) const {
  return (::IsWindow(hwnd))
             ? DesktopWindowTreeHostWin::GetContentWindowForHWND(hwnd)
             : nullptr;
}

////////////////////////////////////////////////////////////////////////////////

display::Screen* CreateDesktopScreen() {
  return new DesktopScreenWin;
}

}  // namespace views
}  // namespace crui
