// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_WIN_TOUCH_INPUT_H_
#define UI_BASE_WIN_TOUCH_INPUT_H_

#include <windows.h>

#include "crui/base/ui_export.h"

namespace crui {

// Wrapper for GetTouchInputInfo, which is not defined before Win7. For
// earlier OS's, this function returns FALSE.
CRUI_EXPORT BOOL GetTouchInputInfoWrapper(HTOUCHINPUT handle,
                                          UINT count,
                                          PTOUCHINPUT pointer,
                                          int size);

}  // namespace crui

#endif  // UI_BASE_WIN_TOUCH_INPUT_H_
