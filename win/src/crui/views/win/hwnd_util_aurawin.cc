// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/win/hwnd_util.h"

///#include "crbase/i18n/rtl.h"
///#include "ui/aura/window.h"
///#include "ui/aura/window_tree_host.h"
#include "crui/views/widget/widget.h"
#include "crui/base/build_platform.h"

namespace crui {
namespace views {

HWND HWNDForView(const View* view) {
  return view->GetWidget() ? HWNDForWidget(view->GetWidget()) : nullptr;
}

HWND HWNDForWidget(const Widget* widget) {
  return HWNDForNativeWindow(widget->GetNativeWindow());
}

HWND HWNDForNativeView(const gfx::NativeView view) {
#if defined(MINI_CHROMIUM_USE_AURA)
  return view && view->GetRootWindow() ? view->GetHost()->GetAcceleratedWidget()
                                       : nullptr;
#else
  return view;
#endif
}

HWND HWNDForNativeWindow(const gfx::NativeWindow window) {
#if defined(MINI_CHROMIUM_USE_AURA)
  return window && window->GetRootWindow()
             ? window->GetHost()->GetAcceleratedWidget()
             : nullptr;
#else
  return window;
#endif
}

gfx::Rect GetWindowBoundsForClientBounds(View* view,
                                         const gfx::Rect& client_bounds) {
  CR_DCHECK(view);
#if defined(MINI_CHROMIUM_USE_AURA)
  aura::WindowTreeHost* host = view->GetWidget()->GetNativeWindow()->GetHost();
  if (host) {
    HWND hwnd = host->GetAcceleratedWidget();
#else
    HWND hwnd = view->GetWidget()->GetNativeWindow();
#endif
    RECT rect = client_bounds.ToRECT();
    DWORD style = ::GetWindowLongW(hwnd, GWL_STYLE);
    DWORD ex_style = ::GetWindowLongW(hwnd, GWL_EXSTYLE);
    AdjustWindowRectEx(&rect, style, FALSE, ex_style);
    return gfx::Rect(rect);
#if defined(MINI_CHROMIUM_USE_AURA)
  }
  return client_bounds;
#endif
}

void ShowSystemMenuAtScreenPixelLocation(HWND window, const gfx::Point& point) {
  UINT flags = TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD;
  ///if (cr::i18n::IsRTL())
  ///  flags |= TPM_RIGHTALIGN;
  HMENU menu = GetSystemMenu(window, FALSE);

  const int command =
      TrackPopupMenu(menu, flags, point.x(), point.y(), 0, window, nullptr);

  if (command)
    ::SendMessageW(window, WM_SYSCOMMAND, command, 0);
}

}  // namespace views
}  // namespace crui
