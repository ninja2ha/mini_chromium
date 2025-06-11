// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/events/event_rewriter.h"

#include <utility>

#include "crui/events/event_rewriter_continuation.h"
#include "crui/events/event_source.h"

namespace crui {

namespace {

crui::EventDispatchDetails DispatcherDestroyed() {
  crui::EventDispatchDetails details;
  details.dispatcher_destroyed = true;
  return details;
}

}  // anonymous namespace

// Temporary fallback implementation, in terms of the old API,
// factored out of EventSource::SendEventToSinkFromRewriter().
// TODO(kpschoedel): Remove along with old API.
EventDispatchDetails EventRewriter::RewriteEvent(
    const Event& event,
    const Continuation continuation) {
  std::unique_ptr<Event> rewritten_event;
  EventRewriteStatus status = RewriteEvent(event, &rewritten_event);
  if (status == EVENT_REWRITE_DISCARD) {
    CR_CHECK(!rewritten_event);
    return continuation->DiscardEvent();
  }
  if (status == EVENT_REWRITE_CONTINUE) {
    CR_CHECK(!rewritten_event);
    return continuation->SendEvent(&event);
  }
  CR_CHECK(rewritten_event);
  EventDispatchDetails details =
      continuation->SendEventFinally(rewritten_event.get());
  while (status == EVENT_REWRITE_DISPATCH_ANOTHER) {
    if (details.dispatcher_destroyed)
      return details;
    std::unique_ptr<Event> new_event;
    status = NextDispatchEvent(*rewritten_event, &new_event);
    if (status == EVENT_REWRITE_DISCARD)
      return continuation->DiscardEvent();
    CR_CHECK(EVENT_REWRITE_CONTINUE != status);
    CR_CHECK(new_event);
    details = continuation->SendEventFinally(new_event.get());
    rewritten_event = std::move(new_event);
  }
  return details;
}

// Temporary default implementation of the old API, so that subclasses'
// implementations can be removed.
// TODO(kpschoedel): Remove old API.
EventRewriteStatus EventRewriter::RewriteEvent(
    const Event& event,
    std::unique_ptr<Event>* rewritten_event) {
  CR_NOTREACHED();
  return EVENT_REWRITE_DISCARD;
}

// Temporary default implementation of the old API, so that subclasses'
// implementations can be removed.
// TODO(kpschoedel): Remove old API.
EventRewriteStatus EventRewriter::NextDispatchEvent(
    const Event& last_event,
    std::unique_ptr<Event>* new_event) {
  CR_NOTREACHED();
  return EVENT_REWRITE_DISCARD;
}

// TODO(kpschoedel): Remove old API.
EventDispatchDetails EventRewriter::SendEventToEventSource(EventSource* source,
                                                           Event* event) const {
  return source->SendEventToSinkFromRewriter(event, this);
}

EventDispatchDetails EventRewriter::SendEvent(const Continuation continuation,
                                              const Event* event) {
  return continuation ? continuation->SendEvent(event) : DispatcherDestroyed();
}

EventDispatchDetails EventRewriter::SendEventFinally(
    const Continuation continuation,
    const Event* event) {
  return continuation ? continuation->SendEventFinally(event)
                      : DispatcherDestroyed();
}

EventDispatchDetails EventRewriter::DiscardEvent(
    const Continuation continuation) {
  return continuation ? continuation->DiscardEvent()
                      : DispatcherDestroyed();
}

}  // namespace crui
