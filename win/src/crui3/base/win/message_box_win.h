// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_WIN_MESSAGE_BOX_WIN_H_
#define UI_BASE_WIN_MESSAGE_BOX_WIN_H_

#include <windows.h>

#include "crbase/strings/string16.h"
#include "crui/base/ui_export.h"

namespace crui {

// A wrapper around Windows' MessageBox function. Using a Chrome specific
// MessageBox function allows us to control certain RTL locale flags so that
// callers don't have to worry about adding these flags when running in a
// right-to-left locale.
CRUI_EXPORT int MessageBox(HWND hwnd,
                           const cr::string16& text,
                           const cr::string16& caption,
                           UINT flags);

}  // namespace crui

#endif  // UI_BASE_WIN_MESSAGE_BOX_WIN_H_
