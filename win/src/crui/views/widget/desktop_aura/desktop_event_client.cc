// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/widget/desktop_aura/desktop_event_client.h"

#include "crui/aura/env.h"

namespace crui {
namespace views {

DesktopEventClient::DesktopEventClient() = default;

DesktopEventClient::~DesktopEventClient() = default;

bool DesktopEventClient::CanProcessEventsWithinSubtree(
    const aura::Window* window) const {
  return true;
}

crui::EventTarget* DesktopEventClient::GetToplevelEventTarget() {
  return aura::Env::GetInstance();
}

}  // namespace views
}  // namespace crui
