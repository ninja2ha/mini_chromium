// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_WM_CORE_COMPOUND_EVENT_FILTER_H_
#define UI_WM_CORE_COMPOUND_EVENT_FILTER_H_

#include "crbase/compiler_specific.h"
#include "crbase/observer_list.h"
#include "crui/events/event.h"
#include "crui/events/event_handler.h"
#include "crui/gfx/native_widget_types.h"
#include "crui/base/ui_export.h"

namespace crui {

class GestureEvent;
class KeyEvent;
class MouseEvent;
class TouchEvent;

namespace aura {
class Window;
}  // namespace aura

namespace wm {

// TODO(beng): This class should die. AddEventHandler() on the root Window
//             should be used instead.
// CompoundEventFilter gets all events first and can provide actions to those
// events. It implements global features such as click to activate a window and
// cursor change when moving mouse.
// Additional event filters can be added to CompoundEventFilter. Events will
// pass through those additional filters in their addition order and could be
// consumed by any of those filters. If an event is consumed by a filter, the
// rest of the filter(s) and CompoundEventFilter will not see the consumed
// event.
class CRUI_EXPORT CompoundEventFilter : public crui::EventHandler {
 public:
  CompoundEventFilter(const CompoundEventFilter&) = delete;
  CompoundEventFilter& operator=(const CompoundEventFilter&) = delete;

  CompoundEventFilter();
  ~CompoundEventFilter() override;

  // Returns the cursor for the specified component.
  static gfx::NativeCursor CursorForWindowComponent(int window_component);

  // Adds/removes additional event filters. This does not take ownership of
  // the EventHandler.
  // NOTE: These handlers are deprecated. Use env::AddPreTargetEventHandler etc.
  // instead.
  void AddHandler(crui::EventHandler* filter);
  void RemoveHandler(crui::EventHandler* filter);

 private:
  // Updates the cursor if the target provides a custom one, and provides
  // default resize cursors for window edges.
  void UpdateCursor(aura::Window* target, crui::MouseEvent* event);

  // Dispatches event to additional filters.
  void FilterKeyEvent(crui::KeyEvent* event);
  void FilterMouseEvent(crui::MouseEvent* event);
  void FilterTouchEvent(crui::TouchEvent* event);

  // Sets the visibility of the cursor if the event is not synthesized.
  void SetCursorVisibilityOnEvent(aura::Window* target,
                                  crui::Event* event,
                                  bool show);

  // Enables or disables mouse events if the event is not synthesized.
  void SetMouseEventsEnableStateOnEvent(aura::Window* target,
                                        crui::Event* event,
                                        bool enable);

  // Overridden from ui::EventHandler:
  void OnKeyEvent(crui::KeyEvent* event) override;
  void OnMouseEvent(crui::MouseEvent* event) override;
  void OnScrollEvent(crui::ScrollEvent* event) override;
  void OnTouchEvent(crui::TouchEvent* event) override;
  void OnGestureEvent(crui::GestureEvent* event) override;

  // Additional pre-target event handlers.
  ///cr::ObserverList<ui::EventHandler, true>::Unchecked handlers_;
  cr::ObserverList<crui::EventHandler, true> handlers_;
};

}  // namespace wm
}  // namespace crui

#endif  // UI_WM_CORE_COMPOUND_EVENT_FILTER_H_
