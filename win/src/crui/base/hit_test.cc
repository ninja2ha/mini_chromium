// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/base/hit_test.h"

#include <set>

#if defined(MINI_CHROMIUM_OS_WIN)
#include <windows.h>
#endif

namespace crui {

bool IsResizingComponent(int component) {
  switch (component) {
    case HTBOTTOM:
    case HTBOTTOMLEFT:
    case HTBOTTOMRIGHT:
    case HTLEFT:
    case HTRIGHT:
    case HTTOP:
    case HTTOPLEFT:
    case HTTOPRIGHT:
      return true;
    default:
      return false;
  }
}

bool CanPerformDragOrResize(int component) {
  return component == HTCAPTION || IsResizingComponent(component);
}

}  // namespace crui
