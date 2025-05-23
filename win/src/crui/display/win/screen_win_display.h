// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_DISPLAY_WIN_SCREEN_WIN_DISPLAY_H_
#define UI_DISPLAY_WIN_SCREEN_WIN_DISPLAY_H_

#include <windows.h>

#include "crui/display/display.h"
#include "crui/gfx/geometry/rect.h"

namespace crui {
namespace display {
namespace win {

class DisplayInfo;

// A display used by ScreenWin.
// It holds a display and additional parameters used for DPI calculations.
class ScreenWinDisplay final {
 public:
  ScreenWinDisplay();
  explicit ScreenWinDisplay(const DisplayInfo& display_info);
  ScreenWinDisplay(const Display& display,
                   const DisplayInfo& display_info);

  const Display& display() const { return display_; }
  const gfx::Rect& pixel_bounds() const { return pixel_bounds_; }
  const gfx::Vector2dF& pixels_per_inch() const { return pixels_per_inch_; }

 private:
  Display display_;
  gfx::Rect pixel_bounds_;
  gfx::Vector2dF pixels_per_inch_;
};

}  // namespace win
}  // namespace display
}  // namespace crui

#endif  // UI_DISPLAY_WIN_SCREEN_WIN_DISPLAY_H_
