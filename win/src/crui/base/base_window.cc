// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/base/base_window.h"

namespace crui {

bool BaseWindow::IsRestored(const BaseWindow& window) {
  return !window.IsMaximized() &&
     !window.IsMinimized() &&
     !window.IsFullscreen();
}

}  // namespace crui

