// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_WM_CORE_DEFAULT_SCREEN_POSITION_CLIENT_H_
#define UI_WM_CORE_DEFAULT_SCREEN_POSITION_CLIENT_H_

#include "crui/aura/client/screen_position_client.h"
#include "crui/base/ui_export.h"

namespace crui {
namespace wm {

// Client that always offsets by the toplevel RootWindow of the passed
// in child NativeWidgetAura.
class CRUI_EXPORT DefaultScreenPositionClient
    : public aura::client::ScreenPositionClient {
 public:
  DefaultScreenPositionClient(const DefaultScreenPositionClient&) = delete;
  DefaultScreenPositionClient& operator=(const DefaultScreenPositionClient&) = delete;

  DefaultScreenPositionClient();
  ~DefaultScreenPositionClient() override;

  // aura::client::ScreenPositionClient overrides:
  void ConvertPointToScreen(const aura::Window* window,
                            gfx::PointF* point) override;
  void ConvertPointFromScreen(const aura::Window* window,
                              gfx::PointF* point) override;
  void ConvertHostPointToScreen(aura::Window* window,
                                gfx::Point* point) override;
  void SetBounds(aura::Window* window,
                 const gfx::Rect& bounds,
                 const display::Display& display) override;

 protected:
  // Returns the origin of the host platform-window in system DIP coordinates.
  virtual gfx::Point GetOriginInScreen(const aura::Window* root_window);
};

}  // namespace wm
}  // namespace crui

#endif  // UI_WM_CORE_DEFAULT_SCREEN_POSITION_CLIENT_H_
