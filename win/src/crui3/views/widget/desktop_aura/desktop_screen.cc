// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/widget/desktop_aura/desktop_screen.h"

#include "crui/display/screen.h"

namespace crui {
namespace views {

void InstallDesktopScreenIfNecessary() {

  // The screen may have already been set in test initialization.
  if (!display::Screen::GetScreen())
    display::Screen::SetScreenInstance(CreateDesktopScreen());
}

}  // namespace views
}  // namespace crui
