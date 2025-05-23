// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_EVENT_CLIENT_H_
#define UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_EVENT_CLIENT_H_

#include "crbase/compiler_specific.h"
#include "crui/aura/client/event_client.h"
#include "crui/base/ui_export.h"

namespace crui {
namespace views {

class CRUI_EXPORT DesktopEventClient : public aura::client::EventClient {
 public:
  DesktopEventClient(const DesktopEventClient&) = delete;
  DesktopEventClient& operator=(const DesktopEventClient&) = delete;

  DesktopEventClient();
  ~DesktopEventClient() override;

  // Overridden from aura::client::EventClient:
  bool CanProcessEventsWithinSubtree(const aura::Window* window) const override;
  crui::EventTarget* GetToplevelEventTarget() override;
};

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_EVENT_CLIENT_H_
