// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_KEYCODES_KEYBOARD_CODES_H_
#define UI_EVENTS_KEYCODES_KEYBOARD_CODES_H_

#include "crui/base/build_platform.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "crui/events/keycodes/keyboard_codes_win.h"
#elif defined(MINI_CHROMIUM_OS_POSIX)
#include "crui/events/keycodes/keyboard_codes_posix.h"
#endif

#endif  // UI_EVENTS_KEYCODES_KEYBOARD_CODES_H_
