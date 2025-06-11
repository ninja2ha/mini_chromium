// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_AURA_CLIENT_EVENT_CLIENT_H_
#define UI_AURA_CLIENT_EVENT_CLIENT_H_

#include "crui/base/ui_export.h"

namespace crui {
class EventTarget;

namespace aura {
class Window;
namespace client {

// An interface implemented by an object that alters event processing.
class CRUI_EXPORT EventClient {
 public:
  // Returns true if events can be processed by |window| or any of its children.
  virtual bool CanProcessEventsWithinSubtree(const Window* window) const = 0;

  // Returns the top level EventTarget for the current environment.
  virtual crui::EventTarget* GetToplevelEventTarget() = 0;

 protected:
  virtual ~EventClient() {}
};

// Sets/Gets the event client on the root Window.
CRUI_EXPORT void SetEventClient(Window* root_window, EventClient* client);
CRUI_EXPORT EventClient* GetEventClient(const Window* root_window);

}  // namespace clients
}  // namespace aura
}  // namespace crui

#endif  // UI_AURA_CLIENT_EVENT_CLIENT_H_
