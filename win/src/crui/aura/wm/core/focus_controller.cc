// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/wm//core/focus_controller.h"

#include "crbase/auto_reset.h"
#include "crui/aura/client/aura_constants.h"
#include "crui/aura/client/capture_client.h"
#include "crui/aura/client/focus_change_observer.h"
#include "crui/aura/env.h"
#include "crui/aura/window_tracker.h"
#include "crui/events/event.h"
#include "crui/aura/wm//core/focus_rules.h"
#include "crui/aura/wm//core/window_util.h"
#include "crui/aura/wm//public/activation_change_observer.h"

namespace crui {
namespace wm {
namespace {

// When a modal window is activated, we bring its entire transient parent chain
// to the front. This function must be called before the modal transient is
// stacked at the top to ensure correct stacking order.
void StackTransientParentsBelowModalWindow(aura::Window* window) {
  if (window->GetProperty(aura::client::kModalKey) != crui::MODAL_TYPE_WINDOW)
    return;

  aura::Window* transient_parent = wm::GetTransientParent(window);
  while (transient_parent) {
    transient_parent->parent()->StackChildAtTop(transient_parent);
    transient_parent = wm::GetTransientParent(transient_parent);
  }
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// FocusController, public:

FocusController::FocusController(FocusRules* rules) : rules_(rules) {
  CR_DCHECK(rules);
}

FocusController::~FocusController() = default;

////////////////////////////////////////////////////////////////////////////////
// FocusController, ActivationClient implementation:

void FocusController::AddObserver(ActivationChangeObserver* observer) {
  activation_observers_.AddObserver(observer);
}

void FocusController::RemoveObserver(ActivationChangeObserver* observer) {
  activation_observers_.RemoveObserver(observer);
}

void FocusController::ActivateWindow(aura::Window* window) {
  FocusWindow(window);
}

void FocusController::DeactivateWindow(aura::Window* window) {
  if (window)
    FocusWindow(rules_->GetNextActivatableWindow(window));
}

const aura::Window* FocusController::GetActiveWindow() const {
  return active_window_;
}

aura::Window* FocusController::GetActivatableWindow(
    aura::Window* window) const {
  return rules_->GetActivatableWindow(window);
}

const aura::Window* FocusController::GetToplevelWindow(
    const aura::Window* window) const {
  return rules_->GetToplevelWindow(window);
}

bool FocusController::CanActivateWindow(const aura::Window* window) const {
  return rules_->CanActivateWindow(window);
}

////////////////////////////////////////////////////////////////////////////////
// FocusController, aura::client::FocusClient implementation:

void FocusController::AddObserver(
    aura::client::FocusChangeObserver* observer) {
  focus_observers_.AddObserver(observer);
}

void FocusController::RemoveObserver(
    aura::client::FocusChangeObserver* observer) {
  focus_observers_.RemoveObserver(observer);
}

void FocusController::FocusWindow(aura::Window* window) {
  FocusAndActivateWindow(
      ActivationChangeObserver::ActivationReason::ACTIVATION_CLIENT, window);
}

void FocusController::ResetFocusWithinActiveWindow(aura::Window* window) {
  CR_DCHECK(window);
  if (!active_window_)
    return;
  if (!active_window_->Contains(window))
    return;
  SetFocusedWindow(window);
}

aura::Window* FocusController::GetFocusedWindow() {
  return focused_window_;
}

////////////////////////////////////////////////////////////////////////////////
// FocusController, ui::EventHandler implementation:
void FocusController::OnKeyEvent(crui::KeyEvent* event) {
}

void FocusController::OnMouseEvent(crui::MouseEvent* event) {
  if (event->type() == crui::ET_MOUSE_PRESSED && !event->handled())
    WindowFocusedFromInputEvent(static_cast<aura::Window*>(event->target()),
                                event);
}

void FocusController::OnScrollEvent(crui::ScrollEvent* event) {
}

void FocusController::OnTouchEvent(crui::TouchEvent* event) {
}

void FocusController::OnGestureEvent(crui::GestureEvent* event) {
  if (event->type() == crui::ET_GESTURE_BEGIN &&
      event->details().touch_points() == 1 &&
      !event->handled()) {
    WindowFocusedFromInputEvent(static_cast<aura::Window*>(event->target()),
                                event);
  }
}

////////////////////////////////////////////////////////////////////////////////
// FocusController, aura::WindowObserver implementation:

void FocusController::OnWindowVisibilityChanged(aura::Window* window,
                                                bool visible) {
  if (!visible)
    WindowLostFocusFromDispositionChange(window, window->parent());
}

void FocusController::OnWindowDestroying(aura::Window* window) {
  // A window's modality state will interfere with focus restoration during its
  // destruction.
  window->ClearProperty(aura::client::kModalKey);
  WindowLostFocusFromDispositionChange(window, window->parent());
}

void FocusController::OnWindowHierarchyChanging(
    const HierarchyChangeParams& params) {
  if (params.receiver == active_window_ &&
      params.target->Contains(params.receiver) && (!params.new_parent ||
      aura::client::GetFocusClient(params.new_parent) !=
          aura::client::GetFocusClient(params.receiver))) {
    WindowLostFocusFromDispositionChange(params.receiver, params.old_parent);
  }
}

void FocusController::OnWindowHierarchyChanged(
    const HierarchyChangeParams& params) {
  if (params.receiver == focused_window_ &&
      params.target->Contains(params.receiver) && (!params.new_parent ||
      aura::client::GetFocusClient(params.new_parent) !=
          aura::client::GetFocusClient(params.receiver))) {
    WindowLostFocusFromDispositionChange(params.receiver, params.old_parent);
  }
}

////////////////////////////////////////////////////////////////////////////////
// FocusController, private:

void FocusController::FocusAndActivateWindow(
    ActivationChangeObserver::ActivationReason reason,
    aura::Window* window) {
  if (window &&
      (window->Contains(focused_window_) || window->Contains(active_window_))) {
    StackActiveWindow();
    return;
  }

  // Focusing a window also activates its containing activatable window. Note
  // that the rules could redirect activation and/or focus.
  aura::Window* focusable = rules_->GetFocusableWindow(window);
  aura::Window* activatable =
      focusable ? rules_->GetActivatableWindow(focusable) : nullptr;

  // We need valid focusable/activatable windows in the event we're not clearing
  // focus. "Clearing focus" is inferred by whether or not |window| passed to
  // this function is non-NULL.
  if (window && (!focusable || !activatable))
    return;
  CR_DCHECK((focusable && activatable) || !window);

  // Activation change observers may change the focused window. If this happens
  // we must not adjust the focus below since this will clobber that change.
  aura::Window* last_focused_window = focused_window_;
  if (!pending_activation_.has_value()) {
    aura::WindowTracker focusable_window_tracker;
    if (focusable) {
      focusable_window_tracker.Add(focusable);
      focusable = nullptr;
    }
    SetActiveWindow(reason, window, activatable);
    if (!focusable_window_tracker.windows().empty())
      focusable = focusable_window_tracker.Pop();
  } else {
    // Only allow the focused window to change, *not* the active window if
    // called reentrantly.
    CR_DCHECK(!activatable || activatable == pending_activation_.value());
  }

  // If the window's ActivationChangeObserver shifted focus to a valid window,
  // we don't want to focus the window we thought would be focused by default.
  if (!updating_focus_) {
    aura::Window* const new_active_window = pending_activation_.has_value()
                                                ? pending_activation_.value()
                                                : active_window_;
    const bool activation_changed_focus =
        last_focused_window != focused_window_;
    if (!activation_changed_focus || !focused_window_) {
      if (new_active_window && focusable)
        CR_DCHECK(new_active_window->Contains(focusable));
      SetFocusedWindow(focusable);
    }
    if (new_active_window && focused_window_)
      CR_DCHECK(new_active_window->Contains(focused_window_));
  }
}

void FocusController::SetFocusedWindow(aura::Window* window) {
  if (updating_focus_ || window == focused_window_)
    return;
  CR_DCHECK(rules_->CanFocusWindow(window, nullptr));
  if (window)
    CR_DCHECK(window == rules_->GetFocusableWindow(window));

  cr::AutoReset<bool> updating_focus(&updating_focus_, true);
  aura::Window* lost_focus = focused_window_;

  // Allow for the window losing focus to be deleted during dispatch. If it is
  // deleted pass NULL to observers instead of a deleted window.
  aura::WindowTracker window_tracker;
  if (lost_focus)
    window_tracker.Add(lost_focus);
  if (focused_window_ && observer_manager_.IsObserving(focused_window_) &&
      focused_window_ != active_window_) {
    observer_manager_.Remove(focused_window_);
  }
  focused_window_ = window;
  if (focused_window_ && !observer_manager_.IsObserving(focused_window_))
    observer_manager_.Add(focused_window_);

  for (auto& observer : focus_observers_) {
    observer.OnWindowFocused(
        focused_window_,
        window_tracker.Contains(lost_focus) ? lost_focus : nullptr);
  }
  if (window_tracker.Contains(lost_focus)) {
    aura::client::FocusChangeObserver* observer =
        aura::client::GetFocusChangeObserver(lost_focus);
    if (observer)
      observer->OnWindowFocused(focused_window_, lost_focus);
  }
  aura::client::FocusChangeObserver* observer =
      aura::client::GetFocusChangeObserver(focused_window_);
  if (observer) {
    observer->OnWindowFocused(
        focused_window_,
        window_tracker.Contains(lost_focus) ? lost_focus : nullptr);
  }
}

void FocusController::SetActiveWindow(
    ActivationChangeObserver::ActivationReason reason,
    aura::Window* requested_window,
    aura::Window* window) {
  if (pending_activation_)
    return;

  if (window == active_window_) {
    if (requested_window) {
      for (auto& observer : activation_observers_)
        observer.OnAttemptToReactivateWindow(requested_window, active_window_);
    }
    return;
  }

  CR_DCHECK(rules_->CanActivateWindow(window));
  if (window)
    CR_DCHECK(window == rules_->GetActivatableWindow(window));

  cr::AutoReset<cr::Optional<aura::Window*>> updating_activation(
      &pending_activation_, cr::make_optional(window));
  aura::Window* lost_activation = active_window_;
  // Allow for the window losing activation to be deleted during dispatch. If
  // it is deleted pass NULL to observers instead of a deleted window.
  aura::WindowTracker window_tracker;
  if (lost_activation)
    window_tracker.Add(lost_activation);

  for (auto& observer : activation_observers_)
    observer.OnWindowActivating(reason, window, active_window_);

  if (active_window_ && observer_manager_.IsObserving(active_window_) &&
      focused_window_ != active_window_) {
    observer_manager_.Remove(active_window_);
  }
  active_window_ = window;
  if (active_window_ && !observer_manager_.IsObserving(active_window_))
    observer_manager_.Add(active_window_);
  if (active_window_)
    StackActiveWindow();

  ActivationChangeObserver* observer = nullptr;
  if (window_tracker.Contains(lost_activation)) {
    observer = GetActivationChangeObserver(lost_activation);
    if (observer)
      observer->OnWindowActivated(reason, active_window_, lost_activation);
  }
  observer = GetActivationChangeObserver(active_window_);
  if (observer) {
    observer->OnWindowActivated(
        reason, active_window_,
        window_tracker.Contains(lost_activation) ? lost_activation : nullptr);
  }
  for (auto& observer : activation_observers_) {
    observer.OnWindowActivated(
        reason, active_window_,
        window_tracker.Contains(lost_activation) ? lost_activation : nullptr);
  }
}

void FocusController::StackActiveWindow() {
  if (active_window_) {
    StackTransientParentsBelowModalWindow(active_window_);
    active_window_->parent()->StackChildAtTop(active_window_);
  }
}

void FocusController::WindowLostFocusFromDispositionChange(
    aura::Window* window,
    aura::Window* next) {
  // TODO(beng): See if this function can be replaced by a call to
  //             FocusWindow().
  // Activation adjustments are handled first in the event of a disposition
  // changed. If an activation change is necessary, focus is reset as part of
  // that process so there's no point in updating focus independently.
  if (window == active_window_) {
    aura::Window* next_activatable = rules_->GetNextActivatableWindow(window);
    SetActiveWindow(
        ActivationChangeObserver::ActivationReason::WINDOW_DISPOSITION_CHANGED,
        nullptr, next_activatable);
    if (!(active_window_ && active_window_->Contains(focused_window_)))
      SetFocusedWindow(next_activatable);
  } else if (window->Contains(focused_window_)) {
    if (pending_activation_) {
      // We're in the process of updating activation, most likely
      // ActivationChangeObserver::OnWindowActivated() is changing something
      // about the focused window (visibility perhaps). Temporarily set the
      // focus to null, we'll set it to something better when activation
      // completes.
      SetFocusedWindow(nullptr);
    } else {
      // Active window isn't changing, but focused window might be.
      SetFocusedWindow(rules_->GetFocusableWindow(next));
    }
  }
}

void FocusController::WindowFocusedFromInputEvent(aura::Window* window,
                                                  const crui::Event* event) {
  // Only focus |window| if it or any of its parents can be focused. Otherwise
  // FocusWindow() will focus the topmost window, which may not be the
  // currently focused one.
  if (rules_->CanFocusWindow(GetToplevelWindow(window), event)) {
    FocusAndActivateWindow(
        ActivationChangeObserver::ActivationReason::INPUT_EVENT, window);
  }
}

}  // namespace wm
}  // namespace crui
