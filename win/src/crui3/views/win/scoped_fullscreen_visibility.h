// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_WIN_SCOPED_FULLSCREEN_VISIBILITY_H_
#define UI_VIEWS_WIN_SCOPED_FULLSCREEN_VISIBILITY_H_

#include <windows.h>

#include <map>

#include "crui/base/ui_export.h"

namespace crui {
namespace views {

// Scoping class that ensures a HWND remains hidden while it enters or leaves
// the fullscreen state. This reduces some flicker-jank that an application UI
// might suffer.
class CRUI_EXPORT ScopedFullscreenVisibility {
 public:
  ScopedFullscreenVisibility(const ScopedFullscreenVisibility&) = delete;
  ScopedFullscreenVisibility& operator=(
      const ScopedFullscreenVisibility&) = delete;

  explicit ScopedFullscreenVisibility(HWND hwnd);
  ~ScopedFullscreenVisibility();

  // Returns true if |hwnd| is currently hidden due to instance(s) of this
  // class.
  static bool IsHiddenForFullscreen(HWND hwnd);

 private:
  using FullscreenHWNDs = std::map<HWND, int>;

  HWND hwnd_;

  static FullscreenHWNDs* full_screen_windows_;
};

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_WIN_SCOPED_FULLSCREEN_VISIBILITY_H_
