// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/display/screen.h"

#include "crbase/build_platform.h"
#include "crui/display/display.h"
#include "crui/display/types/display_constants.h"
#include "crui/gfx/geometry/rect.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "crui/display/win/screen_win.h"
#endif

namespace crui {
namespace display {

namespace {

Screen* g_screen = nullptr;

}  // namespace

Screen::Screen() : display_id_for_new_windows_(kInvalidDisplayId) {}

Screen::~Screen() = default;

// static
Screen* Screen::GetScreen() {
#if defined(MINI_CHROMIUM_OS_MACOSX)
  // TODO(scottmg): https://crbug.com/558054
  if (!g_screen)
    g_screen = CreateNativeScreen();
#endif

#if defined(MINI_CHROMIUM_OS_WIN)
  if (!g_screen)
    return win::ScreenWin::GetInstance();
#endif

  return g_screen;
}

// static
void Screen::SetScreenInstance(Screen* instance) {
  g_screen = instance;
}

Display Screen::GetDisplayNearestView(gfx::NativeView view) const {
  return GetDisplayNearestWindow(GetWindowForView(view));
}

display::Display Screen::GetDisplayForNewWindows() const {
  display::Display display;
  if (GetDisplayWithDisplayId(display_id_for_new_windows_, &display))
    return display;

  // Fallback to primary display.
  return GetPrimaryDisplay();
}

void Screen::SetDisplayForNewWindows(int64_t display_id) {
  // GetDisplayForNewWindows() handles invalid display ids.
  display_id_for_new_windows_ = display_id;
}

gfx::Rect Screen::ScreenToDIPRectInWindow(gfx::NativeView view,
                                          const gfx::Rect& screen_rect) const {
  float scale = GetDisplayNearestView(view).device_scale_factor();
  return ScaleToEnclosingRect(screen_rect, 1.0f / scale);
}

gfx::Rect Screen::DIPToScreenRectInWindow(gfx::NativeView view,
                                          const gfx::Rect& dip_rect) const {
  float scale = GetDisplayNearestView(view).device_scale_factor();
  return ScaleToEnclosingRect(dip_rect, scale);
}

bool Screen::GetDisplayWithDisplayId(int64_t display_id,
                                     Display* display) const {
  for (const Display& display_in_list : GetAllDisplays()) {
    if (display_in_list.id() == display_id) {
      *display = display_in_list;
      return true;
    }
  }
  return false;
}

void Screen::SetPanelRotationForTesting(int64_t display_id,
                                        Display::Rotation rotation) {
  // Not implemented.
  CR_DCHECK(false);
}

// static 
gfx::NativeWindow Screen::GetWindowForView(gfx::NativeView view) {
  return view;
}

}  // namespace display
}  // namespace crui
