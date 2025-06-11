// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_WIN_HWND_METRICS_H_
#define UI_BASE_WIN_HWND_METRICS_H_

#include <windows.h>

#include "crui/base/ui_export.h"

namespace crui {

// The size, in pixels, of the non-client frame around a window on |monitor|.
CRUI_EXPORT int GetFrameThickness(HMONITOR monitor);

}  // namespace crui

#endif  // UI_BASE_WIN_HWND_METRICS_H_
