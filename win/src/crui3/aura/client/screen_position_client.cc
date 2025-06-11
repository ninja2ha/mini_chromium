// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/client/screen_position_client.h"

#include "crui/base/class_property.h"
#include "crui/gfx/geometry/point_conversions.h"

DEFINE_UI_CLASS_PROPERTY_TYPE(crui::aura::client::ScreenPositionClient*)

namespace crui {

namespace aura {
namespace client {

DEFINE_UI_CLASS_PROPERTY_KEY(ScreenPositionClient*,
                             kScreenPositionClientKey,
                             nullptr)

void ScreenPositionClient::ConvertPointToScreen(const Window* window,
                                                gfx::Point* point) {
  gfx::PointF point_float(*point);
  ConvertPointToScreen(window, &point_float);
  *point = gfx::ToFlooredPoint(point_float);
}

void ScreenPositionClient::ConvertPointFromScreen(const Window* window,
                                                  gfx::Point* point) {
  gfx::PointF point_float(*point);
  ConvertPointFromScreen(window, &point_float);
  *point = gfx::ToFlooredPoint(point_float);
}

void SetScreenPositionClient(Window* root_window,
                             ScreenPositionClient* client) {
  CR_DCHECK(root_window->GetRootWindow() == root_window);
  root_window->SetProperty(kScreenPositionClientKey, client);
}

ScreenPositionClient* GetScreenPositionClient(const Window* root_window) {
  if (root_window)
    CR_DCHECK(root_window->GetRootWindow() == root_window);
  return root_window ?
      root_window->GetProperty(kScreenPositionClientKey) : NULL;
}

}  // namespace client
}  // namespace aura
}  // namespace crui
