// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/wm//core/compound_event_filter.h"

#include "crbase/logging.h"
#include "crui/aura/client/cursor_client.h"
#include "crui/aura/client/drag_drop_client.h"
#include "crui/aura/env.h"
#include "crui/aura/window.h"
#include "crui/aura/window_delegate.h"
#include "crui/aura/window_event_dispatcher.h"
#include "crui/base/hit_test.h"
#include "crui/events/event.h"
#include "crui/aura/wm//public/activation_client.h"
#include "crui/base/build_platform.h"

namespace crui {
namespace wm {

namespace {

// Returns true if the cursor should be hidden on touch events.
// TODO(tdanderson|rsadam): Move this function into CursorClient.
bool ShouldHideCursorOnTouch(const crui::TouchEvent& event) {
#if defined(MINI_CHROMIUM_OS_WIN)
  return true;
#else
  // Linux Aura does not hide the cursor on touch by default.
  // TODO(tdanderson): Change this if having consistency across
  // all platforms which use Aura is desired.
  return false;
#endif
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// CompoundEventFilter, public:

CompoundEventFilter::CompoundEventFilter() {
}

CompoundEventFilter::~CompoundEventFilter() {
  // Additional filters are not owned by CompoundEventFilter and they
  // should all be removed when running here. |handlers_| has
  // check_empty == true and will DCHECK failure if it is not empty.
}

// static
gfx::NativeCursor CompoundEventFilter::CursorForWindowComponent(
    int window_component) {
  switch (window_component) {
    case HTBOTTOM:
      return crui::CursorType::kSouthResize;
    case HTBOTTOMLEFT:
      return crui::CursorType::kSouthWestResize;
    case HTBOTTOMRIGHT:
      return crui::CursorType::kSouthEastResize;
    case HTLEFT:
      return crui::CursorType::kWestResize;
    case HTRIGHT:
      return crui::CursorType::kEastResize;
    case HTTOP:
      return crui::CursorType::kNorthResize;
    case HTTOPLEFT:
      return crui::CursorType::kNorthWestResize;
    case HTTOPRIGHT:
      return crui::CursorType::kNorthEastResize;
    default:
      return crui::CursorType::kNull;
  }
}

void CompoundEventFilter::AddHandler(crui::EventHandler* handler) {
  handlers_.AddObserver(handler);
}

void CompoundEventFilter::RemoveHandler(crui::EventHandler* handler) {
  handlers_.RemoveObserver(handler);
}

////////////////////////////////////////////////////////////////////////////////
// CompoundEventFilter, private:

void CompoundEventFilter::UpdateCursor(aura::Window* target,
                                       crui::MouseEvent* event) {
  // If drag and drop is in progress, let the drag drop client set the cursor
  // instead of setting the cursor here.
  aura::Window* root_window = target->GetRootWindow();
  aura::client::DragDropClient* drag_drop_client =
      aura::client::GetDragDropClient(root_window);
  if (drag_drop_client && drag_drop_client->IsDragDropInProgress())
    return;

  aura::client::CursorClient* cursor_client =
      aura::client::GetCursorClient(root_window);
  if (cursor_client) {
    gfx::NativeCursor cursor = target->GetCursor(event->location());
    if ((event->flags() & crui::EF_IS_NON_CLIENT)) {
      if (target->delegate()) {
        int window_component =
            target->delegate()->GetNonClientComponent(event->location());
        cursor = CursorForWindowComponent(window_component);
      } else {
        // Allow the OS to handle non client cursors if we don't have a
        // a delegate to handle the non client hittest.
        return;
      }
    }
    cursor_client->SetCursor(cursor);
  }
}

void CompoundEventFilter::FilterKeyEvent(crui::KeyEvent* event) {
  for (crui::EventHandler& handler : handlers_) {
    if (event->stopped_propagation())
      break;
    handler.OnKeyEvent(event);
  }
}

void CompoundEventFilter::FilterMouseEvent(crui::MouseEvent* event) {
  for (crui::EventHandler& handler : handlers_) {
    if (event->stopped_propagation())
      break;
    handler.OnMouseEvent(event);
  }
}

void CompoundEventFilter::FilterTouchEvent(crui::TouchEvent* event) {
  for (crui::EventHandler& handler : handlers_) {
    if (event->stopped_propagation())
      break;
    handler.OnTouchEvent(event);
  }
}

void CompoundEventFilter::SetCursorVisibilityOnEvent(aura::Window* target,
                                                     crui::Event* event,
                                                     bool show) {
  if (event->flags() & crui::EF_IS_SYNTHESIZED)
    return;

  aura::client::CursorClient* client =
      aura::client::GetCursorClient(target->GetRootWindow());
  if (!client)
    return;

  if (show)
    client->ShowCursor();
  else
    client->HideCursor();
}

void CompoundEventFilter::SetMouseEventsEnableStateOnEvent(aura::Window* target,
                                                           crui::Event* event,
                                                           bool enable) {
  if (event->flags() & crui::EF_IS_SYNTHESIZED)
    return;
  aura::client::CursorClient* client =
      aura::client::GetCursorClient(target->GetRootWindow());
  if (!client)
    return;

  if (enable)
    client->EnableMouseEvents();
  else
    client->DisableMouseEvents();
}

////////////////////////////////////////////////////////////////////////////////
// CompoundEventFilter, ui::EventHandler implementation:

void CompoundEventFilter::OnKeyEvent(crui::KeyEvent* event) {
  aura::Window* target = static_cast<aura::Window*>(event->target());
  aura::client::CursorClient* client =
      aura::client::GetCursorClient(target->GetRootWindow());
  if (client && client->ShouldHideCursorOnKeyEvent(*event))
    SetCursorVisibilityOnEvent(target, event, false);

  FilterKeyEvent(event);
}

void CompoundEventFilter::OnMouseEvent(crui::MouseEvent* event) {
  aura::Window* window = static_cast<aura::Window*>(event->target());

  // We must always update the cursor, otherwise the cursor can get stuck if an
  // event filter registered with us consumes the event.
  // It should also update the cursor for clicking and wheels for ChromeOS boot.
  // When ChromeOS is booted, it hides the mouse cursor but immediate mouse
  // operation will show the cursor.
  // We also update the cursor for mouse enter in case a mouse cursor is sent to
  // outside of the root window and moved back for some reasons (e.g. running on
  // on Desktop for testing, or a bug in pointer barrier).
  if (!(event->flags() & crui::EF_FROM_TOUCH) &&
       (event->type() == crui::ET_MOUSE_ENTERED ||
        event->type() == crui::ET_MOUSE_MOVED ||
        event->type() == crui::ET_MOUSE_PRESSED ||
        event->type() == crui::ET_MOUSEWHEEL)) {
    SetMouseEventsEnableStateOnEvent(window, event, true);
    SetCursorVisibilityOnEvent(window, event, true);
    UpdateCursor(window, event);
  }

  FilterMouseEvent(event);
}

void CompoundEventFilter::OnScrollEvent(crui::ScrollEvent* event) {
}

void CompoundEventFilter::OnTouchEvent(crui::TouchEvent* event) {
  FilterTouchEvent(event);
  if (!event->handled() && event->type() == crui::ET_TOUCH_PRESSED &&
      ShouldHideCursorOnTouch(*event)) {
    aura::Window* target = static_cast<aura::Window*>(event->target());
    CR_DCHECK(target);
    if (!aura::Env::GetInstance()->IsMouseButtonDown())
      SetMouseEventsEnableStateOnEvent(target, event, false);
  }
}

void CompoundEventFilter::OnGestureEvent(crui::GestureEvent* event) {
  for (crui::EventHandler& handler : handlers_) {
    if (event->stopped_propagation())
      break;
    handler.OnGestureEvent(event);
  }
}

}  // namespace wm
}  // namespace crui
