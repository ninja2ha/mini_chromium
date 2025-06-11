// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/events/event_processor.h"

#include "crui/events/event_target.h"
#include "crui/events/event_targeter.h"

namespace crui {

EventProcessor::EventProcessor() {}

EventProcessor::~EventProcessor() {}

EventDispatchDetails EventProcessor::OnEventFromSource(Event* event) {
  cr::WeakPtr<EventProcessor> weak_this = weak_ptr_factory_.GetWeakPtr();
  // If |event| is in the process of being dispatched or has already been
  // dispatched, then dispatch a copy of the event instead. We expect event
  // target to be already set if event phase is after EP_PREDISPATCH.
  bool dispatch_original_event = event->phase() == EP_PREDISPATCH;
  CR_DCHECK(dispatch_original_event || event->target());
  Event* event_to_dispatch = event;
  std::unique_ptr<Event> event_copy;
  if (!dispatch_original_event) {
    event_copy = Event::Clone(*event);
    event_to_dispatch = event_copy.get();
  }

  EventDispatchDetails details;
  OnEventProcessingStarted(event_to_dispatch);
  if (!event_to_dispatch->handled()) {
    EventTarget* target = nullptr;
    EventTarget* root = GetRootForEvent(event_to_dispatch);
    CR_DCHECK(root);
    EventTargeter* targeter = root->GetEventTargeter();
    if (targeter) {
      target = targeter->FindTargetForEvent(root, event_to_dispatch);
    } else {
      targeter = GetDefaultEventTargeter();
      if (event_to_dispatch->target())
        target = root;
      else
        target = targeter->FindTargetForEvent(root, event_to_dispatch);
    }
    CR_DCHECK(targeter);

    while (target) {
      details = DispatchEvent(target, event_to_dispatch);

      if (!dispatch_original_event) {
        if (event_to_dispatch->stopped_propagation())
          event->StopPropagation();
        else if (event_to_dispatch->handled())
          event->SetHandled();
      }

      if (details.dispatcher_destroyed)
        return details;

      if (!weak_this) {
        details.dispatcher_destroyed = true;
        return details;
      }

      if (details.target_destroyed || event->handled() || !target)
        break;

      CR_DCHECK(targeter);
      target = targeter->FindNextBestTarget(target, event_to_dispatch);
    }
  }
  OnEventProcessingFinished(event);
  return details;
}

void EventProcessor::OnEventProcessingStarted(Event* event) {
}

void EventProcessor::OnEventProcessingFinished(Event* event) {
}

}  // namespace cr
