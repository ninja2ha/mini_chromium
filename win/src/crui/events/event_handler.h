// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_EVENT_HANDLER_H_
#define UI_EVENTS_EVENT_HANDLER_H_

#include <vector>

#include "crbase/containers/stack.h"
#include "crui/events/event_constants.h"
#include "crui/base/ui_export.h"

namespace crui {

class CancelModeEvent;
class Event;
class EventDispatcher;
class EventTarget;
class GestureEvent;
class KeyEvent;
class MouseEvent;
class ScrollEvent;
class TouchEvent;

// Dispatches events to appropriate targets.  The default implementations of
// all of the specific handlers (e.g. OnKeyEvent, OnMouseEvent) do nothing.
class CRUI_EXPORT EventHandler {
 public:
  EventHandler(const EventHandler&) = delete;
  EventHandler& operator=(const EventHandler&) = delete;
  EventHandler();

  virtual ~EventHandler();

  // Disables a CHECK() that this has been removed from all pre-target
  // handlers in the destructor.
  // TODO(sky): remove, used to track https://crbug.com/867035.
  static void DisableCheckTargets() { check_targets_ = false; }

  // This is called for all events. The default implementation routes the event
  // to one of the event-specific callbacks (OnKeyEvent, OnMouseEvent etc.). If
  // this is overridden, then normally, the override should chain into the
  // default implementation for un-handled events.
  virtual void OnEvent(Event* event);

  virtual void OnKeyEvent(KeyEvent* event);

  virtual void OnMouseEvent(MouseEvent* event);

  virtual void OnScrollEvent(ScrollEvent* event);

  virtual void OnTouchEvent(TouchEvent* event);

  virtual void OnGestureEvent(GestureEvent* event);

  virtual void OnCancelMode(CancelModeEvent* event);

 private:
  friend class EventDispatcher;
  friend class EventTarget;

  // EventDispatcher pushes itself on the top of this stack while dispatching
  // events to this then pops itself off when done.
  cr::stack<EventDispatcher*> dispatchers_;

  // Set of EventTargets |this| has been installed as a pre-target handler on.
  // This is a vector as AddPreTargetHandler() may be called multiple times for
  // the same EventTarget.
  // TODO(sky): remove, used to track https://crbug.com/867035.
  std::vector<EventTarget*> targets_installed_on_;

  static bool check_targets_;
};

typedef std::vector<EventHandler*> EventHandlerList;

}  // namespace crui

#endif  // UI_EVENTS_EVENT_HANDLER_H_
