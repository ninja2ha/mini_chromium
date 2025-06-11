// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/window_layout_manager.h"

#include "crui/aura/window.h"

namespace crui {
namespace aura {

WindowLayoutManager::WindowLayoutManager() {
}

WindowLayoutManager::~WindowLayoutManager() {
}

void WindowLayoutManager::SetChildBoundsDirect(aura::Window* child,
                                               const gfx::Rect& bounds) {
  child->SetBoundsInternal(bounds);
}

}  // namespace aura
}  // namespace crui
