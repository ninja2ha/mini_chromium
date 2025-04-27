// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_EVENT_SINK_H_
#define UI_EVENTS_EVENT_SINK_H_

#include "crui/events/event_dispatcher.h"

namespace crui {

class Event;

// EventSink receives events from an EventSource.
class CRUI_EXPORT EventSink {
 public:
  virtual ~EventSink() {}

  // Receives events from EventSource.
  virtual EventDispatchDetails OnEventFromSource(Event* event)
      CR_WARN_UNUSED_RESULT = 0;
};

}  // namespace crui

#endif  // UI_EVENTS_EVENT_SINK_H_
