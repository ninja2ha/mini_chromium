// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/client/capture_client.h"

#include "crui/aura/window.h"
#include "crui/aura/window_event_dispatcher.h"
#include "crui/base/class_property.h"

DEFINE_UI_CLASS_PROPERTY_TYPE(aura::client::CaptureClient*)

namespace crui {
namespace aura {
namespace client {

DEFINE_UI_CLASS_PROPERTY_KEY(CaptureClient*, kRootWindowCaptureClientKey, NULL)

void SetCaptureClient(Window* root_window, CaptureClient* client) {
  root_window->SetProperty(kRootWindowCaptureClientKey, client);
}

CaptureClient* GetCaptureClient(Window* root_window) {
  return root_window ?
      root_window->GetProperty(kRootWindowCaptureClientKey) : NULL;
}

Window* GetCaptureWindow(Window* window) {
  Window* root_window = window->GetRootWindow();
  if (!root_window)
    return NULL;
  CaptureClient* capture_client = GetCaptureClient(root_window);
  return capture_client ? capture_client->GetCaptureWindow() : NULL;
}

}  // namespace client
}  // namespace aura
}  // namespace crui
