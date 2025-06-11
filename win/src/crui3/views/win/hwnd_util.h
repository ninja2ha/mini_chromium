// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_WIN_HWND_UTIL_H_
#define UI_VIEWS_WIN_HWND_UTIL_H_

#include "crui/gfx/geometry/point.h"
#include "crui/gfx/geometry/rect.h"
#include "crui/gfx/native_widget_types.h"
#include "crui/base/ui_export.h"

namespace crui {
namespace views {

class View;
class Widget;

// Returns the HWND for the specified View.
CRUI_EXPORT HWND HWNDForView(const View* view);

// Returns the HWND for the specified Widget.
CRUI_EXPORT HWND HWNDForWidget(const Widget* widget);

// Returns the HWND for the specified NativeView.
CRUI_EXPORT HWND HWNDForNativeView(const gfx::NativeView view);

// Returns the HWND for the specified NativeWindow.
CRUI_EXPORT HWND HWNDForNativeWindow(const gfx::NativeWindow window);

CRUI_EXPORT gfx::Rect GetWindowBoundsForClientBounds(
    View* view, const gfx::Rect& client_bounds);

// Shows |window|'s system menu (at a specified |point| in screen physical
// coordinates).
CRUI_EXPORT void ShowSystemMenuAtScreenPixelLocation(HWND window,
                                                     const gfx::Point& point);

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_WIN_HWND_UTIL_H_
