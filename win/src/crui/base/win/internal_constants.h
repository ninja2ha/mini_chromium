// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_WIN_INTERNAL_CONSTANTS_H_
#define UI_BASE_WIN_INTERNAL_CONSTANTS_H_

#include "crui/base/ui_export.h"

namespace crui {

// This window property if set on the window does not activate the window for a
// touch based WM_MOUSEACTIVATE message.
CRUI_EXPORT extern const wchar_t kIgnoreTouchMouseActivateForWindow[];

// This property is put on an HWND so the compositor output knows to treat it
// as transparent and draw to it using WS_EX_LAYERED (if using the software
// compositor).
CRUI_EXPORT extern const wchar_t kWindowTranslucent[];

}  // namespace crui

#endif  // UI_BASE_WIN_INTERNAL_CONSTANTS_H_


