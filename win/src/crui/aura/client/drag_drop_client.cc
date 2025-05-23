// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/client/drag_drop_client.h"

#include "crui/aura/window.h"
#include "crui/base/class_property.h"

DEFINE_UI_CLASS_PROPERTY_TYPE(crui::aura::client::DragDropClient*)

namespace crui {
namespace aura {
namespace client {

DEFINE_UI_CLASS_PROPERTY_KEY(DragDropClient*,
                             kRootWindowDragDropClientKey,
                             nullptr)

void SetDragDropClient(Window* root_window, DragDropClient* client) {
  CR_DCHECK(root_window->GetRootWindow() == root_window);
  root_window->SetProperty(kRootWindowDragDropClientKey, client);
}

DragDropClient* GetDragDropClient(Window* root_window) {
  if (root_window)
    CR_DCHECK(root_window->GetRootWindow() == root_window);
  return root_window ?
      root_window->GetProperty(kRootWindowDragDropClientKey) : NULL;
}

}  // namespace client
}  // namespace aura
}  // namespace crui
