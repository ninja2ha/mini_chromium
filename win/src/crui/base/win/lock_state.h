// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_WIN_LOCK_STATE_H_
#define UI_BASE_WIN_LOCK_STATE_H_

#include "crui/base/ui_export.h"

namespace crui {

// Returns true if the screen is currently locked.
CRUI_EXPORT bool IsWorkstationLocked();

}  // namespace crui

#endif  // UI_BASE_WIN_LOCK_STATE_H_
