// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_KEYCODES_KEYBOARD_CODE_CONVERSION_WIN_H_
#define UI_EVENTS_KEYCODES_KEYBOARD_CODE_CONVERSION_WIN_H_

#include "crui/base/ui_export.h"
#include "crui/events/keycodes/dom/dom_code.h"
#include "crui/events/keycodes/keyboard_codes.h"

namespace crui {

// Methods to convert ui::KeyboardCode/Windows virtual key type methods.
CRUI_EXPORT WORD WindowsKeyCodeForKeyboardCode(KeyboardCode keycode);
CRUI_EXPORT KeyboardCode KeyboardCodeForWindowsKeyCode(WORD keycode);
CRUI_EXPORT DomCode CodeForWindowsScanCode(WORD scan_code);

}  // namespace crui

#endif  // UI_EVENTS_KEYCODES_KEYBOARD_CODE_CONVERSION_WIN_H_
