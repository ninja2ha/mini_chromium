// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_WIN_SYSTEM_EVENT_STATE_LOOKUP_H_
#define UI_EVENTS_WIN_SYSTEM_EVENT_STATE_LOOKUP_H_

#include "crui/base/ui_export.h"

namespace crui {
namespace win {

// Returns true if the Shift key is currently pressed.
CRUI_EXPORT bool IsShiftPressed();

// Returns true if either Control key is pressed (including due to AltGraph).
CRUI_EXPORT bool IsCtrlPressed();

// Returns true if either Alt key is currently pressed.
CRUI_EXPORT bool IsAltPressed();

// Returns true if the AltRight (i.e. either Alt or AltGraph) key is pressed.
// This is used in events_win.cc to detect the physical AltGraph key.
CRUI_EXPORT bool IsAltRightPressed();

// Returns true if the Windows key is currently pressed.
CRUI_EXPORT bool IsWindowsKeyPressed();

// Returns true if the caps lock state is on.
CRUI_EXPORT bool IsCapsLockOn();

// Returns true if the num lock state is on.
CRUI_EXPORT bool IsNumLockOn();

// Returns true if the scroll lock state is on.
CRUI_EXPORT bool IsScrollLockOn();

}  // namespace win
}  // namespace crui

#endif  // UI_EVENTS_WIN_SYSTEM_EVENT_STATE_LOOKUP_H_
