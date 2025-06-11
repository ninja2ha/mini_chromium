// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/client/cursor_client.h"

#include "crui/aura/window.h"
#include "crui/base/class_property.h"

DEFINE_UI_CLASS_PROPERTY_TYPE(crui::aura::client::CursorClient*)

namespace crui {

namespace aura {
namespace client {

// A property key to store a client that handles window moves.
DEFINE_UI_CLASS_PROPERTY_KEY(CursorClient*, kCursorClientKey, nullptr)

void SetCursorClient(Window* window, CursorClient* client) {
  window->SetProperty(kCursorClientKey, client);
}

CursorClient* GetCursorClient(Window* window) {
  return window->GetProperty(kCursorClientKey);
}

}  // namespace client
}  // namespace aura
}  // namespace crui
