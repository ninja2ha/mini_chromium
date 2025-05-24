// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/client/window_parenting_client.h"

#include "crui/aura/env.h"
#include "crui/aura/window.h"
#include "crui/aura/window_event_dispatcher.h"
#include "crui/base/class_property.h"

DEFINE_UI_CLASS_PROPERTY_TYPE(crui::aura::client::WindowParentingClient*)

namespace crui {

namespace aura {
namespace client {

DEFINE_UI_CLASS_PROPERTY_KEY(WindowParentingClient*,
                             kRootWindowWindowParentingClientKey,
                             NULL)

void SetWindowParentingClient(Window* window,
                              WindowParentingClient* window_tree_client) {
  CR_DCHECK(window);

  Window* root_window = window->GetRootWindow();
  CR_DCHECK(root_window);
  root_window->SetProperty(kRootWindowWindowParentingClientKey,
                           window_tree_client);
}

WindowParentingClient* GetWindowParentingClient(Window* window) {
  CR_DCHECK(window);
  Window* root_window = window->GetRootWindow();
  CR_DCHECK(root_window);
  WindowParentingClient* client =
      root_window->GetProperty(kRootWindowWindowParentingClientKey);
  CR_DCHECK(client);
  return client;
}

void ParentWindowWithContext(Window* window,
                             Window* context,
                             const gfx::Rect& screen_bounds) {
  CR_DCHECK(context);

  // |context| must be attached to a hierarchy with a WindowParentingClient.
  WindowParentingClient* client = GetWindowParentingClient(context);
  CR_DCHECK(client);
  Window* default_parent = client->GetDefaultParent(window, screen_bounds);
  default_parent->AddChild(window);
}

}  // namespace client
}  // namespace aura
}  // namespace crui
