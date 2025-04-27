// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_NULL_EVENT_TARGETER_H_
#define UI_EVENTS_NULL_EVENT_TARGETER_H_

#include "crbase/compiler_specific.h"
#include "crui/events/event_targeter.h"
#include "crui/base/ui_export.h"

namespace crui {

// NullEventTargeter can be installed on a root window to prevent it from
// dispatching events such as during shutdown.
class CRUI_EXPORT NullEventTargeter : public EventTargeter {
 public:
  NullEventTargeter(const NullEventTargeter&) = delete;
  NullEventTargeter& operator=(const NullEventTargeter&) = delete;
  NullEventTargeter();
  ~NullEventTargeter() override;

  // EventTargeter:
  EventTarget* FindTargetForEvent(EventTarget* root, Event* event) override;
  EventTarget* FindNextBestTarget(EventTarget* previous_target,
                                  Event* event) override;
};

}  // namespace crui

#endif  // UI_EVENTS_NULL_EVENT_TARGETER_H_
