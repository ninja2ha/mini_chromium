// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_AURA_EVENT_INJECTOR_H_
#define UI_AURA_EVENT_INJECTOR_H_

#include "crui/base/ui_export.h"

namespace crui {
class Event;
struct EventDispatchDetails;

namespace aura {

class WindowTreeHost;

// Used to inject events as if they came from the OS.
class CRUI_EXPORT EventInjector {
 public:
  EventInjector(const EventInjector&) = delete;
  EventInjector& operator=(const EventInjector&) = delete;

  EventInjector();
  ~EventInjector();

  // Inject |event| to |host|. If |event| is a LocatedEvent, then coordinates
  // are relative to host and in DIPs.
  crui::EventDispatchDetails Inject(WindowTreeHost* host, crui::Event* event);
};

}  // namespace aura
}  // namespace crui

#endif  // UI_AURA_EVENT_INJECTOR_H_
