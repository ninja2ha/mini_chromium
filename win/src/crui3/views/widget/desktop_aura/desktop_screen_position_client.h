// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_SCREEN_POSITION_CLIENT_H_
#define UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_SCREEN_POSITION_CLIENT_H_

#include "crui/aura/wm/core/default_screen_position_client.h"
#include "crui/base/ui_export.h"

namespace crui {
namespace views {

// Client that always offsets by the toplevel RootWindow of the passed
// in child NativeWidgetAura.
class CRUI_EXPORT DesktopScreenPositionClient
    : public wm::DefaultScreenPositionClient {
 public:
  DesktopScreenPositionClient(const DesktopScreenPositionClient&) = delete;
  DesktopScreenPositionClient& operator=(
      const DesktopScreenPositionClient&) = delete;

  explicit DesktopScreenPositionClient(aura::Window* root_window);
  ~DesktopScreenPositionClient() override;

  // aura::client::DefaultScreenPositionClient:
  void SetBounds(aura::Window* window,
                 const gfx::Rect& bounds,
                 const display::Display& display) override;

 private:
  aura::Window* root_window_;
};

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_SCREEN_POSITION_CLIENT_H_
