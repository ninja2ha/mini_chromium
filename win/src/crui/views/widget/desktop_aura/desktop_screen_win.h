// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_SCREEN_WIN_H_
#define UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_SCREEN_WIN_H_

#include "crui/display/win/screen_win.h"
#include "crui/base/ui_export.h"

namespace crui {
namespace views {

class CRUI_EXPORT DesktopScreenWin : public display::win::ScreenWin {
public:
  DesktopScreenWin(const DesktopScreenWin&) = delete;
  DesktopScreenWin& operator=(const DesktopScreenWin&) = delete;

  DesktopScreenWin();
  ~DesktopScreenWin() override;

 private:
  // Overridden from display::win::ScreenWin:
  display::Display GetDisplayMatching(
      const gfx::Rect& match_rect) const override;
  HWND GetHWNDFromNativeView(gfx::NativeView window) const override;
  gfx::NativeWindow GetNativeWindowFromHWND(HWND hwnd) const override;
};

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_SCREEN_WIN_H_
