// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_ACCELERATORS_MENU_LABEL_ACCELERATOR_UTIL_H_
#define UI_BASE_ACCELERATORS_MENU_LABEL_ACCELERATOR_UTIL_H_

#include <string>

#include "crbase/strings/string16.h"
#include "crui/base/ui_export.h"

namespace crui {

CRUI_EXPORT cr::char16 GetMnemonic(const cr::string16& label);

// This function escapes every '&' in label by replacing it with '&&', to avoid
// having single ampersands in user-provided strings treated as accelerators.
CRUI_EXPORT cr::string16 EscapeMenuLabelAmpersands(
    const cr::string16& label);

}  // namespace ui

#endif  // UI_BASE_ACCELERATORS_MENU_LABEL_ACCELERATOR_UTIL_H_
