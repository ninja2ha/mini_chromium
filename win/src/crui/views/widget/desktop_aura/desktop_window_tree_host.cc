// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/widget/desktop_aura/desktop_window_tree_host.h"

#include "crui/aura/window.h"
#include "crui/aura/window_tree_host.h"
#include "crui/display/screen.h"
#include "crui/views/widget/desktop_aura/desktop_screen_position_client.h"

namespace crui {
namespace views {

void DesktopWindowTreeHost::SetBoundsInDIP(const gfx::Rect& bounds) {
  aura::Window* root = AsWindowTreeHost()->window();
  const gfx::Rect bounds_in_pixels =
      display::Screen::GetScreen()->DIPToScreenRectInWindow(root, bounds);
  AsWindowTreeHost()->SetBoundsInPixels(bounds_in_pixels);
}

std::unique_ptr<aura::client::ScreenPositionClient>
DesktopWindowTreeHost::CreateScreenPositionClient() {
  return std::make_unique<DesktopScreenPositionClient>(
      AsWindowTreeHost()->window());
}

}  // namespace views
}  // namespace crui
