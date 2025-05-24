// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/client/event_client.h"

#include "crui/aura/window.h"
#include "crui/aura/window_event_dispatcher.h"
#include "crui/base/class_property.h"

DEFINE_UI_CLASS_PROPERTY_TYPE(aura::client::EventClient*)

namespace crui {

namespace aura {
namespace client {

DEFINE_UI_CLASS_PROPERTY_KEY(EventClient*, kRootWindowEventClientKey, NULL)

void SetEventClient(Window* root_window, EventClient* client) {
  CR_DCHECK(root_window->GetRootWindow() == root_window);
  root_window->SetProperty(kRootWindowEventClientKey, client);
}

EventClient* GetEventClient(const Window* root_window) {
  if (root_window)
    CR_DCHECK(root_window->GetRootWindow() == root_window);
  return root_window ?
      root_window->GetProperty(kRootWindowEventClientKey) : NULL;
}

}  // namespace client
}  // namespace aura

}  // namespace crui
