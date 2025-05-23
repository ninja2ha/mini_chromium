// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/window_event_dispatcher.h"

#include <stddef.h>

#include <utility>

#include "crbase/functional/bind.h"
#include "crbase/logging.h"
#include "crbase/threading/thread_task_runner_handle.h"
///#include "crbase/trace_event/trace_event.h"
#include "crui/gfx/geometry/transform.h"
#include "crui/aura/client/capture_client.h"
#include "crui/aura/client/cursor_client.h"
#include "crui/aura/client/event_client.h"
#include "crui/aura/client/focus_client.h"
#include "crui/aura/client/screen_position_client.h"
#include "crui/aura/env.h"
#include "crui/aura/env_input_state_controller.h"
#include "crui/aura/window_delegate.h"
#include "crui/aura/window_event_dispatcher_observer.h"
#include "crui/aura/window_targeter.h"
#include "crui/aura/window_tracker.h"
#include "crui/aura/window_tree_host.h"
#include "crui/base/hit_test.h"
///#include "crui/base/ime/input_method.h"
///#include "crui/compositor/dip_util.h"
#include "crui/display/screen.h"
#include "crui/events/event.h"
#include "crui/events/event_utils.h"
#include "crui/events/gestures/gesture_recognizer.h"
#include "crui/events/gestures/gesture_types.h"
#include "crui/events/platform/platform_event_source.h"

typedef crui::EventDispatchDetails DispatchDetails;

namespace crui {
namespace aura {

namespace {

// Returns true if |target| has a non-client (frame) component at |location|,
// in window coordinates.
bool IsNonClientLocation(Window* target, const gfx::Point& location) {
  if (!target->delegate())
    return false;
  int hit_test_code = target->delegate()->GetNonClientComponent(location);
  return hit_test_code != HTCLIENT && hit_test_code != HTNOWHERE;
}

Window* ConsumerToWindow(crui::GestureConsumer* consumer) {
  return consumer ? static_cast<Window*>(consumer) : NULL;
}

bool IsEventCandidateForHold(const crui::Event& event) {
  if (event.type() == crui::ET_TOUCH_MOVED)
    return true;
  if (event.type() == crui::ET_MOUSE_DRAGGED)
    return true;
  if (event.IsMouseEvent() && (event.flags() & crui::EF_IS_SYNTHESIZED))
    return true;
  return false;
}

}  // namespace

WindowEventDispatcher::ObserverNotifier::ObserverNotifier(
    WindowEventDispatcher* dispatcher,
    const crui::Event& event)
    : dispatcher_(dispatcher) {
  for (WindowEventDispatcherObserver& observer :
       Env::GetInstance()->window_event_dispatcher_observers()) {
    observer.OnWindowEventDispatcherStartedProcessing(dispatcher, event);
  }
}

WindowEventDispatcher::ObserverNotifier::~ObserverNotifier() {
  for (WindowEventDispatcherObserver& observer :
       Env::GetInstance()->window_event_dispatcher_observers()) {
    observer.OnWindowEventDispatcherFinishedProcessingEvent(dispatcher_);
  }
}

////////////////////////////////////////////////////////////////////////////////
// WindowEventDispatcher, public:

WindowEventDispatcher::WindowEventDispatcher(WindowTreeHost* host)
    : host_(host),
      observer_manager_(this),
      event_targeter_(std::make_unique<WindowTargeter>()) {
  Env::GetInstance()->gesture_recognizer()->AddGestureEventHelper(this);
  Env::GetInstance()->AddObserver(this);
}

WindowEventDispatcher::~WindowEventDispatcher() {
  ///TRACE_EVENT0("shutdown", "WindowEventDispatcher::Destructor");
  Env::GetInstance()->gesture_recognizer()->RemoveGestureEventHelper(this);
  Env::GetInstance()->RemoveObserver(this);
}

void WindowEventDispatcher::Shutdown() {
  in_shutdown_ = true;
}

crui::EventTargeter* WindowEventDispatcher::GetDefaultEventTargeter() {
  return event_targeter_.get();
}

void WindowEventDispatcher::RepostEvent(const crui::LocatedEvent* event) {
  CR_DCHECK(event->type() == crui::ET_MOUSE_PRESSED ||
            event->type() == crui::ET_GESTURE_TAP_DOWN ||
            event->type() == crui::ET_TOUCH_PRESSED);
  // We allow for only one outstanding repostable event. This is used
  // in exiting context menus.  A dropped repost request is allowed.
  if (event->type() == crui::ET_MOUSE_PRESSED) {
    held_repostable_event_ = std::make_unique<crui::MouseEvent>(
        *event->AsMouseEvent(), static_cast<aura::Window*>(event->target()),
        window());
  } else if (event->type() == crui::ET_TOUCH_PRESSED) {
    held_repostable_event_ =
        std::make_unique<crui::TouchEvent>(*event->AsTouchEvent());
  } else {
    CR_DCHECK(event->type() == crui::ET_GESTURE_TAP_DOWN);
    held_repostable_event_.reset();
    // TODO(rbyers): Reposing of gestures is tricky to get
    // right, so it's not yet supported.  crbug.com/170987.
  }

  if (held_repostable_event_) {
    cr::ThreadTaskRunnerHandle::Get()->PostNonNestableTask(
        CR_FROM_HERE,
        cr::BindOnce(
            cr::IgnoreResult(&WindowEventDispatcher::DispatchHeldEvents),
            repost_event_factory_.GetWeakPtr()));
  }
}

void WindowEventDispatcher::OnMouseEventsEnableStateChanged(bool enabled) {
  // Send entered / exited so that visual state can be updated to match
  // mouse events state.
  PostSynthesizeMouseMove();
  // TODO(mazda): Add code to disable mouse events when |enabled| == false.
}

void WindowEventDispatcher::DispatchCancelModeEvent() {
  crui::CancelModeEvent event;
  Window* focused_window = client::GetFocusClient(window())->GetFocusedWindow();
  if (focused_window && !window()->Contains(focused_window))
    focused_window = NULL;
  DispatchDetails details =
      DispatchEvent(focused_window ? focused_window : window(), &event);
  if (details.dispatcher_destroyed)
    return;
}

void WindowEventDispatcher::DispatchGestureEvent(
    crui::GestureConsumer* raw_input_consumer,
    crui::GestureEvent* event) {
  DispatchDetails details = DispatchHeldEvents();
  if (details.dispatcher_destroyed)
    return;
  Window* target = ConsumerToWindow(raw_input_consumer);
  if (target) {
    event->ConvertLocationToTarget(window(), target);
    details = DispatchEvent(target, event);
    if (details.dispatcher_destroyed)
      return;
  }
}

DispatchDetails WindowEventDispatcher::DispatchMouseExitAtPoint(
    Window* window,
    const gfx::Point& point,
    int event_flags) {
  crui::MouseEvent event(crui::ET_MOUSE_EXITED, point, point, 
                         crui::EventTimeForNow(),
                         event_flags, crui::EF_NONE);
  return DispatchMouseEnterOrExit(window, event, crui::ET_MOUSE_EXITED);
}

void WindowEventDispatcher::ProcessedTouchEvent(
    uint32_t unique_event_id,
    Window* window,
    crui::EventResult result,
    bool is_source_touch_event_set_non_blocking) {
  crui::GestureRecognizer::Gestures gestures =
      Env::GetInstance()->gesture_recognizer()->AckTouchEvent(
          unique_event_id, result, is_source_touch_event_set_non_blocking,
          window);
  DispatchDetails details = ProcessGestures(window, std::move(gestures));
  if (details.dispatcher_destroyed)
    return;
}

void WindowEventDispatcher::HoldPointerMoves() {
  if (!move_hold_count_)
    held_event_factory_.InvalidateWeakPtrs();
  ++move_hold_count_;
  ///TRACE_EVENT_ASYNC_BEGIN0("ui", "WindowEventDispatcher::HoldPointerMoves",
  ///                         this);
}

void WindowEventDispatcher::ReleasePointerMoves() {
  --move_hold_count_;
  CR_DCHECK(move_hold_count_ >= 0);
  if (!move_hold_count_) {
    if (held_move_event_) {
      // We don't want to call DispatchHeldEvents directly, because this might
      // be called from a deep stack while another event, in which case
      // dispatching another one may not be safe/expected.  Instead we post a
      // task, that we may cancel if HoldPointerMoves is called again before it
      // executes.
      cr::ThreadTaskRunnerHandle::Get()->PostNonNestableTask(
          CR_FROM_HERE,
          cr::BindOnce(
              cr::IgnoreResult(&WindowEventDispatcher::DispatchHeldEvents),
              held_event_factory_.GetWeakPtr()));
    } else {
      if (did_dispatch_held_move_event_callback_)
        std::move(did_dispatch_held_move_event_callback_).Run();
    }
  }
  ///TRACE_EVENT_ASYNC_END0("ui", "WindowEventDispatcher::HoldPointerMoves", this);
}

gfx::Point WindowEventDispatcher::GetLastMouseLocationInRoot() const {
  gfx::Point location = Env::GetInstance()->last_mouse_location();
  ConvertPointFromScreen(&location);
  return location;
}

void WindowEventDispatcher::OnHostLostMouseGrab() {
  mouse_pressed_handler_ = NULL;
  mouse_moved_handler_ = NULL;
}

void WindowEventDispatcher::OnCursorMovedToRootLocation(
    const gfx::Point& root_location) {
  Env::GetInstance()->env_controller()->SetLastMouseLocation(window(),
                                                             root_location);

  // Synthesize a mouse move in case the cursor's location in root coordinates
  // changed but its position in WindowTreeHost coordinates did not.
  PostSynthesizeMouseMove();
}

void WindowEventDispatcher::OnPostNotifiedWindowDestroying(Window* window) {
  OnWindowHidden(window, WINDOW_DESTROYED);
}

////////////////////////////////////////////////////////////////////////////////
// WindowEventDispatcher, private:

Window* WindowEventDispatcher::window() {
  return host_->window();
}

const Window* WindowEventDispatcher::window() const {
  return host_->window();
}

void WindowEventDispatcher::ConvertPointFromScreen(gfx::Point* point) const {
  client::ScreenPositionClient* client =
      client::GetScreenPositionClient(window());
  if (client)
    client->ConvertPointFromScreen(window(), point);
}

void WindowEventDispatcher::TransformEventForDeviceScaleFactor(
    crui::LocatedEvent* event) {
  event->UpdateForRootTransform(
      host_->GetInverseRootTransform(),
      host_->GetInverseRootTransformForLocalEventCoordinates());
}

void WindowEventDispatcher::DispatchMouseExitToHidingWindow(Window* window) {
  // Dispatching events during shutdown can cause crashes (e.g. in Chrome OS
  // system tray cleanup). https://crbug.com/874156
  if (in_shutdown_)
    return;

  // The mouse capture is intentionally ignored. Think that a mouse enters
  // to a window, the window sets the capture, the mouse exits the window,
  // and then it releases the capture. In that case OnMouseExited won't
  // be called. So it is natural not to emit OnMouseExited even though
  // |window| is the capture window.
  gfx::Point last_mouse_location = GetLastMouseLocationInRoot();
  if (window->Contains(mouse_moved_handler_) &&
      window->ContainsPointInRoot(last_mouse_location)) {
    DispatchDetails details =
        DispatchMouseExitAtPoint(this->window(), last_mouse_location);
    if (details.dispatcher_destroyed)
      return;
  }
}

crui::EventDispatchDetails WindowEventDispatcher::DispatchMouseEnterOrExit(
    Window* target,
    const crui::MouseEvent& event,
    crui::EventType type) {
  Env::GetInstance()->env_controller()->UpdateStateForMouseEvent(window(),
                                                                 event);
  if (!mouse_moved_handler_ || !mouse_moved_handler_->HasTargetHandler() ||
      !window()->Contains(mouse_moved_handler_))
    return DispatchDetails();

  // |event| may be an event in the process of being dispatched to a target (in
  // which case its locations will be in the event's target's coordinate
  // system), or a synthetic event created in root-window (in which case, the
  // event's target will be NULL, and the event will be in the root-window's
  // coordinate system.
  if (!target)
    target = window();
  crui::MouseEvent translated_event(event,
                                    target,
                                    mouse_moved_handler_,
                                    type,
                                    event.flags() | crui::EF_IS_SYNTHESIZED);
  return DispatchEvent(mouse_moved_handler_, &translated_event);
}

crui::EventDispatchDetails WindowEventDispatcher::ProcessGestures(
    Window* target,
    crui::GestureRecognizer::Gestures gestures) {
  DispatchDetails details;
  if (gestures.empty())
    return details;

  // If a window has been hidden between the touch event and now, the associated
  // gestures may not have a valid target.
  if (!target)
    return details;

  for (const auto& event : gestures) {
    event->ConvertLocationToTarget(window(), target);
    details = DispatchEvent(target, event.get());
    if (details.dispatcher_destroyed || details.target_destroyed)
      break;
  }
  return details;
}

void WindowEventDispatcher::OnWindowHidden(Window* invisible,
                                           WindowHiddenReason reason) {
  // If the window the mouse was pressed in becomes invisible, it should no
  // longer receive mouse events.
  if (invisible->Contains(mouse_pressed_handler_))
    mouse_pressed_handler_ = NULL;
  if (invisible->Contains(mouse_moved_handler_))
    mouse_moved_handler_ = NULL;
  if (invisible->Contains(touchpad_pinch_handler_))
    touchpad_pinch_handler_ = nullptr;

  // If events are being dispatched from a nested message-loop, and the target
  // of the outer loop is hidden or moved to another dispatcher during
  // dispatching events in the inner loop, then reset the target for the outer
  // loop.
  if (invisible->Contains(old_dispatch_target_))
    old_dispatch_target_ = NULL;

  invisible->CleanupGestureState();

  // Do not clear the capture, and the |event_dispatch_target_| if the
  // window is moving across hosts, because the target itself is actually still
  // visible and clearing them stops further event processing, which can cause
  // unexpected behaviors. See crbug.com/157583
  if (reason != WINDOW_MOVING) {
    // We don't ask |invisible| here, because invisible may have been removed
    // from the window hierarchy already by the time this function is called
    // (OnWindowDestroyed).
    client::CaptureClient* capture_client =
        client::GetCaptureClient(host_->window());
    Window* capture_window =
        capture_client ? capture_client->GetCaptureWindow() : NULL;

    if (invisible->Contains(event_dispatch_target_))
      event_dispatch_target_ = NULL;

    // If the ancestor of the capture window is hidden, release the capture.
    // Note that this may delete the window so do not use capture_window
    // after this.
    if (invisible->Contains(capture_window) && invisible != window())
      capture_window->ReleaseCapture();
  }
}

bool WindowEventDispatcher::is_dispatched_held_event(
    const crui::Event& event) const {
  return dispatching_held_event_ == &event;
}

////////////////////////////////////////////////////////////////////////////////
// WindowEventDispatcher, aura::client::CaptureDelegate implementation:

void WindowEventDispatcher::UpdateCapture(Window* old_capture,
                                          Window* new_capture) {
  // |mouse_moved_handler_| may have been set to a Window in a different root
  // (see below). Clear it here to ensure we don't end up referencing a stale
  // Window.
  if (mouse_moved_handler_ && !window()->Contains(mouse_moved_handler_))
    mouse_moved_handler_ = NULL;

  if (old_capture && old_capture->GetRootWindow() == window() &&
      old_capture->delegate()) {
    // Send a capture changed event with the most recent mouse screen location.
    const gfx::Point location = Env::GetInstance()->last_mouse_location();
    crui::MouseEvent event(crui::ET_MOUSE_CAPTURE_CHANGED, location, location,
                           crui::EventTimeForNow(), 0, 0);

    DispatchDetails details = DispatchEvent(old_capture, &event);
    if (details.dispatcher_destroyed)
      return;

    if (!details.target_destroyed)
      old_capture->delegate()->OnCaptureLost();
  }

  if (new_capture) {
    // Make all subsequent mouse events go to the capture window. We shouldn't
    // need to send an event here as OnCaptureLost() should take care of that.
    if (mouse_moved_handler_ || Env::GetInstance()->IsMouseButtonDown())
      mouse_moved_handler_ = new_capture;
  } else {
    // Make sure mouse_moved_handler gets updated.
    DispatchDetails details = SynthesizeMouseMoveEvent();
    if (details.dispatcher_destroyed)
      return;
  }
  mouse_pressed_handler_ = NULL;
}

void WindowEventDispatcher::OnOtherRootGotCapture() {
  // Windows provides the TrackMouseEvents API which allows us to rely on the
  // OS to send us the mouse exit events (WM_MOUSELEAVE). Additionally on
  // desktop Windows, every top level window could potentially have its own
  // root window, in which case this function will get called whenever those
  // windows grab mouse capture. Sending mouse exit messages in these cases
  // causes subtle bugs like (crbug.com/394672).
#if !defined(MINI_CHROMIUM_OS_WIN)
  if (mouse_moved_handler_) {
    // Dispatch a mouse exit to reset any state associated with hover. This is
    // important when going from no window having capture to a window having
    // capture because we do not dispatch ET_MOUSE_CAPTURE_CHANGED in this case.
    DispatchDetails details =
        DispatchMouseExitAtPoint(nullptr, GetLastMouseLocationInRoot());
    if (details.dispatcher_destroyed)
      return;
  }
#endif

  mouse_moved_handler_ = NULL;
  mouse_pressed_handler_ = NULL;
}

void WindowEventDispatcher::SetNativeCapture() {
  host_->SetCapture();
}

void WindowEventDispatcher::ReleaseNativeCapture() {
  host_->ReleaseCapture();
}

////////////////////////////////////////////////////////////////////////////////
// WindowEventDispatcher, ui::EventProcessor implementation:

crui::EventTarget* WindowEventDispatcher::GetRootForEvent(crui::Event* event) {
  return window();
}

void WindowEventDispatcher::OnEventProcessingStarted(crui::Event* event) {
  // Don't dispatch events during shutdown.
  if (in_shutdown_) {
    event->SetHandled();
    return;
  }

  // The held events are already in |window()|'s coordinate system. So it is
  // not necessary to apply the transform to convert from the host's
  // coordinate system to |window()|'s coordinate system.
  if (event->IsLocatedEvent() && !is_dispatched_held_event(*event))
    TransformEventForDeviceScaleFactor(
        static_cast<crui::LocatedEvent*>(event));

  observer_notifiers_.push(std::make_unique<ObserverNotifier>(this, *event));
}

void WindowEventDispatcher::OnEventProcessingFinished(crui::Event* event) {
  if (in_shutdown_)
    return;

  observer_notifiers_.pop();
}

////////////////////////////////////////////////////////////////////////////////
// WindowEventDispatcher, ui::EventDispatcherDelegate implementation:

bool WindowEventDispatcher::CanDispatchToTarget(crui::EventTarget* target) {
  return event_dispatch_target_ == target;
}

crui::EventDispatchDetails WindowEventDispatcher::PreDispatchEvent(
    crui::EventTarget* target,
    crui::Event* event) {
  Window* target_window = static_cast<Window*>(target);
  CR_CHECK(window()->Contains(target_window));

  if (!(event->flags() & crui::EF_IS_SYNTHESIZED)) {
    ///fraction_of_time_without_user_input_recorder_.RecordEventAtTime(
    ///    event->time_stamp());
  }

  if (!dispatching_held_event_) {
    bool can_be_held = IsEventCandidateForHold(*event);
    if (!move_hold_count_ || !can_be_held) {
      if (can_be_held)
        held_move_event_.reset();
      DispatchDetails details = DispatchHeldEvents();
      if (details.dispatcher_destroyed || details.target_destroyed)
        return details;
    }
  }

  DispatchDetails details;
  if (event->IsMouseEvent()) {
    details = PreDispatchMouseEvent(target_window, event->AsMouseEvent());
  } else if (event->IsScrollEvent()) {
    details = PreDispatchLocatedEvent(target_window, event->AsScrollEvent());
  } else if (event->IsTouchEvent()) {
    details = PreDispatchTouchEvent(target_window, event->AsTouchEvent());
  } else if (event->IsKeyEvent()) {
    details = PreDispatchKeyEvent(event->AsKeyEvent());
  } else if (event->IsPinchEvent()) {
    details = PreDispatchPinchEvent(target_window, event->AsGestureEvent());
  }
  if (details.dispatcher_destroyed || details.target_destroyed)
    return details;

  old_dispatch_target_ = event_dispatch_target_;
  event_dispatch_target_ = target_window;
  return DispatchDetails();
}

crui::EventDispatchDetails WindowEventDispatcher::PostDispatchEvent(
    crui::EventTarget* target,
    const crui::Event& event) {
  DispatchDetails details;
  if (!target || target != event_dispatch_target_)
    details.target_destroyed = true;
  event_dispatch_target_ = old_dispatch_target_;
  old_dispatch_target_ = NULL;
#ifndef NDEBUG
  CR_DCHECK(!event_dispatch_target_ || 
            window()->Contains(event_dispatch_target_));
#endif

  if (event.IsTouchEvent() && !details.target_destroyed) {
    // Do not let 'held' touch events contribute to any gestures unless it is
    // being dispatched.
    if (is_dispatched_held_event(event) || !held_move_event_ ||
        !held_move_event_->IsTouchEvent()) {
      const crui::TouchEvent& touchevent = *event.AsTouchEvent();
  
      if (!touchevent.synchronous_handling_disabled()) {
        Window* window = static_cast<Window*>(target);
        crui::GestureRecognizer::Gestures gestures =
            Env::GetInstance()->gesture_recognizer()->AckTouchEvent(
                touchevent.unique_event_id(), event.result(),
                false /* is_source_touch_event_set_non_blocking */, window);
  
        return ProcessGestures(window, std::move(gestures));
      }
    }
  }

  return details;
}

////////////////////////////////////////////////////////////////////////////////
// WindowEventDispatcher, ui::GestureEventHelper implementation:

bool WindowEventDispatcher::CanDispatchToConsumer(
    crui::GestureConsumer* consumer) {
  Window* consumer_window = ConsumerToWindow(consumer);
  return (consumer_window && consumer_window->GetRootWindow() == window());
}

void WindowEventDispatcher::DispatchSyntheticTouchEvent(crui::TouchEvent* event) {
  // The synthetic event's location is based on the last known location of
  // the pointer, in dips. OnEventFromSource expects events with co-ordinates
  // in raw pixels, so we convert back to raw pixels here.
  CR_DCHECK(event->type() == crui::ET_TOUCH_CANCELLED ||
            event->type() == crui::ET_TOUCH_PRESSED);
  event->UpdateForRootTransform(
      host_->GetRootTransform(),
      host_->GetRootTransformForLocalEventCoordinates());
  DispatchDetails details = OnEventFromSource(event);
  if (details.dispatcher_destroyed)
    return;
}

////////////////////////////////////////////////////////////////////////////////
// WindowEventDispatcher, WindowObserver implementation:

void WindowEventDispatcher::OnWindowDestroying(Window* window) {
  if (!host_->window()->Contains(window))
    return;

  SynthesizeMouseMoveAfterChangeToWindow(window);
}

void WindowEventDispatcher::OnWindowDestroyed(Window* window) {
  // We observe all windows regardless of what root Window (if any) they're
  // attached to.
  observer_manager_.Remove(window);

  // In theory this should be cleaned up by other checks, but we are getting
  // crashes that seem to indicate otherwise. See https://crbug.com/942552 for
  // one case.
  if (window == mouse_moved_handler_)
    mouse_moved_handler_ = nullptr;
}

void WindowEventDispatcher::OnWindowAddedToRootWindow(Window* attached) {
  if (!observer_manager_.IsObserving(attached))
    observer_manager_.Add(attached);

  if (!host_->window()->Contains(attached))
    return;

  SynthesizeMouseMoveAfterChangeToWindow(attached);
}

void WindowEventDispatcher::OnWindowRemovingFromRootWindow(Window* detached,
                                                           Window* new_root) {
  if (!host_->window()->Contains(detached))
    return;

  CR_DCHECK(client::GetCaptureWindow(window()) != window());

  DispatchMouseExitToHidingWindow(detached);
  SynthesizeMouseMoveAfterChangeToWindow(detached);

  // Hiding the window releases capture which can implicitly destroy the window
  // so the window may no longer be valid after this call.
  OnWindowHidden(detached, new_root ? WINDOW_MOVING : WINDOW_HIDDEN);
}

void WindowEventDispatcher::OnWindowVisibilityChanging(Window* window,
                                                       bool visible) {
  if (!host_->window()->Contains(window))
    return;

  DispatchMouseExitToHidingWindow(window);
}

void WindowEventDispatcher::OnWindowVisibilityChanged(Window* window,
                                                      bool visible) {
  if (!host_->window()->Contains(window))
    return;

  if (window->ContainsPointInRoot(GetLastMouseLocationInRoot()))
    PostSynthesizeMouseMove();

  // Hiding the window releases capture which can implicitly destroy the window
  // so the window may no longer be valid after this call.
  if (!visible)
    OnWindowHidden(window, WINDOW_HIDDEN);
}

void WindowEventDispatcher::OnWindowBoundsChanged(
    Window* window,
    const gfx::Rect& old_bounds,
    const gfx::Rect& new_bounds,
    crui::PropertyChangeReason reason) {
  if (!host_->window()->Contains(window))
    return;

  if (window == host_->window()) {
    ///TRACE_EVENT1("ui", "WindowEventDispatcher::OnWindowBoundsChanged(root)",
    ///             "size", new_bounds.size().ToString());

    DispatchDetails details = DispatchHeldEvents();
    if (details.dispatcher_destroyed)
      return;

    synthesize_mouse_move_ = false;
  }

  if (window->IsVisible() &&
      window->event_targeting_policy() != EventTargetingPolicy::kNone) {
    gfx::Rect old_bounds_in_root = old_bounds, new_bounds_in_root = new_bounds;
    Window::ConvertRectToTarget(window->parent(), host_->window(),
                                &old_bounds_in_root);
    Window::ConvertRectToTarget(window->parent(), host_->window(),
                                &new_bounds_in_root);
    gfx::Point last_mouse_location = GetLastMouseLocationInRoot();
    if (old_bounds_in_root.Contains(last_mouse_location) !=
        new_bounds_in_root.Contains(last_mouse_location)) {
      PostSynthesizeMouseMove();
    }
  }
}

void WindowEventDispatcher::OnWindowTargetTransformChanging(
    Window* window,
    const gfx::Transform& new_transform) {
  window_transforming_ = true;
  if (!synthesize_mouse_move_ && host_->window()->Contains(window))
    SynthesizeMouseMoveAfterChangeToWindow(window);
}

void WindowEventDispatcher::OnWindowTransformed(
    Window* window,
    crui::PropertyChangeReason reason) {
  // Call SynthesizeMouseMoveAfterChangeToWindow() only if it's the first time
  // that OnWindowTransformed() is called after
  // OnWindowTargetTransformChanging() (to avoid generating multiple mouse
  // events during animation).
  if (window_transforming_ && !synthesize_mouse_move_ &&
      host_->window()->Contains(window)) {
    SynthesizeMouseMoveAfterChangeToWindow(window);
  }
  window_transforming_ = false;
}

///////////////////////////////////////////////////////////////////////////////
// WindowEventDispatcher, EnvObserver implementation:

void WindowEventDispatcher::OnWindowInitialized(Window* window) {
  observer_manager_.Add(window);
}

////////////////////////////////////////////////////////////////////////////////
// WindowEventDispatcher, private:

crui::EventDispatchDetails WindowEventDispatcher::DispatchHeldEvents() {
  if (!held_repostable_event_ && !held_move_event_) {
    if (did_dispatch_held_move_event_callback_)
      std::move(did_dispatch_held_move_event_callback_).Run();
    return DispatchDetails();
  }

  CR_CHECK(!dispatching_held_event_);

  DispatchDetails dispatch_details;
  if (held_repostable_event_) {
    if (held_repostable_event_->type() == crui::ET_MOUSE_PRESSED ||
        held_repostable_event_->type() == crui::ET_TOUCH_PRESSED) {
      std::unique_ptr<crui::LocatedEvent> event =
          std::move(held_repostable_event_);
      dispatching_held_event_ = event.get();
      dispatch_details = OnEventFromSource(event.get());
    } else {
      // TODO(rbyers): GESTURE_TAP_DOWN not yet supported: crbug.com/170987.
      CR_NOTREACHED();
    }
    if (dispatch_details.dispatcher_destroyed)
      return dispatch_details;
  }

  if (held_move_event_) {
    // |held_move_event_| should be cleared here. Some event handler can
    // create its own run loop on an event (e.g. WindowMove loop for
    // tab-dragging), which means the other move events need to be processed
    // before this OnEventFromSource() finishes. See also b/119260190.
    std::unique_ptr<crui::LocatedEvent> event = std::move(held_move_event_);

    // If a mouse move has been synthesized, the target location is suspect,
    // so drop the held mouse event.
    if (event->IsTouchEvent() ||
        (event->IsMouseEvent() && !synthesize_mouse_move_)) {
      dispatching_held_event_ = event.get();
      dispatch_details = OnEventFromSource(event.get());
    }
  }

  if (!dispatch_details.dispatcher_destroyed) {
    dispatching_held_event_ = nullptr;
    for (WindowEventDispatcherObserver& observer :
         Env::GetInstance()->window_event_dispatcher_observers()) {
      observer.OnWindowEventDispatcherDispatchedHeldEvents(this);
    }
    if (did_dispatch_held_move_event_callback_)
      std::move(did_dispatch_held_move_event_callback_).Run();
  }

  return dispatch_details;
}

void WindowEventDispatcher::PostSynthesizeMouseMove() {
  // No one should care where the real mouse is when this flag is on. So there
  // is no need to send a synthetic mouse move here.
  if (crui::PlatformEventSource::ShouldIgnoreNativePlatformEvents())
    return;

  if (synthesize_mouse_move_ || in_shutdown_)
    return;
  synthesize_mouse_move_ = true;
  cr::ThreadTaskRunnerHandle::Get()->PostNonNestableTask(
      CR_FROM_HERE,
      cr::BindOnce(
          cr::IgnoreResult(&WindowEventDispatcher::SynthesizeMouseMoveEvent),
          held_event_factory_.GetWeakPtr()));
}

void WindowEventDispatcher::SynthesizeMouseMoveAfterChangeToWindow(
    Window* window) {
  if (in_shutdown_)
    return;
  if (window->IsVisible() &&
      window->ContainsPointInRoot(GetLastMouseLocationInRoot())) {
    PostSynthesizeMouseMove();
  }
}

crui::EventDispatchDetails WindowEventDispatcher::SynthesizeMouseMoveEvent() {
  DispatchDetails details;
  if (!synthesize_mouse_move_ || in_shutdown_)
    return details;
  synthesize_mouse_move_ = false;

  // No need to generate mouse event if the cursor is invisible and not locked.
  client::CursorClient* cursor_client =
      client::GetCursorClient(host_->window());
  if (cursor_client && (!cursor_client->IsMouseEventsEnabled() ||
                        (!cursor_client->IsCursorVisible() &&
                         !cursor_client->IsCursorLocked()))) {
    return details;
  }

  // If one of the mouse buttons is currently down, then do not synthesize a
  // mouse-move event. In such cases, aura could synthesize a DRAGGED event
  // instead of a MOVED event, but in multi-display/multi-host scenarios, the
  // DRAGGED event can be synthesized in the incorrect host. So avoid
  // synthesizing any events at all.
  if (Env::GetInstance()->mouse_button_flags())
    return details;

  // Do not use GetLastMouseLocationInRoot here because it's not updated when
  // the mouse is not over the window or when the window is minimized.
  gfx::Point mouse_location =
      display::Screen::GetScreen()->GetCursorScreenPoint();
  ConvertPointFromScreen(&mouse_location);
  if (!window()->bounds().Contains(mouse_location))
    return details;
  gfx::Point host_mouse_location = mouse_location;
  host_->ConvertDIPToPixels(&host_mouse_location);
  crui::MouseEvent event(crui::ET_MOUSE_MOVED, host_mouse_location,
                         host_mouse_location, crui::EventTimeForNow(),
                         crui::EF_IS_SYNTHESIZED, 0);
  return OnEventFromSource(&event);
}

DispatchDetails WindowEventDispatcher::PreDispatchLocatedEvent(
    Window* target,
    crui::LocatedEvent* event) {
  int flags = event->flags();
  if (IsNonClientLocation(target, event->location()))
    flags |= crui::EF_IS_NON_CLIENT;
  event->set_flags(flags);

  if (!is_dispatched_held_event(*event) &&
      (event->IsMouseEvent() || event->IsScrollEvent()) &&
      !(event->flags() & crui::EF_IS_SYNTHESIZED)) {
    synthesize_mouse_move_ = false;
  }

  return DispatchDetails();
}

DispatchDetails WindowEventDispatcher::PreDispatchMouseEvent(
    Window* target,
    crui::MouseEvent* event) {
  client::CursorClient* cursor_client = client::GetCursorClient(window());
  // We allow synthesized mouse exit events through even if mouse events are
  // disabled. This ensures that hover state, etc on controls like buttons is
  // cleared.
  if (cursor_client &&
      !cursor_client->IsMouseEventsEnabled() &&
      (event->flags() & crui::EF_IS_SYNTHESIZED) &&
      (event->type() != crui::ET_MOUSE_EXITED)) {
    event->SetHandled();
    return DispatchDetails();
  }

  Env::GetInstance()->env_controller()->UpdateStateForMouseEvent(window(),
                                                                 *event);

  if (IsEventCandidateForHold(*event) && !dispatching_held_event_) {
    if (move_hold_count_) {
      held_move_event_ =
          std::make_unique<crui::MouseEvent>(*event, target, window());
      event->SetHandled();
      return DispatchDetails();
    } else {
      // We may have a held event for a period between the time move_hold_count_
      // fell to 0 and the DispatchHeldEvents executes. Since we're going to
      // dispatch the new event directly below, we can reset the old one.
      held_move_event_.reset();
    }
  }

  switch (event->type()) {
    case crui::ET_MOUSE_EXITED:
      if (!target || target == window()) {
        DispatchDetails details =
            DispatchMouseEnterOrExit(target, *event, crui::ET_MOUSE_EXITED);
        if (details.dispatcher_destroyed) {
          event->SetHandled();
          return details;
        }
        mouse_moved_handler_ = NULL;
      }
      break;
    case crui::ET_MOUSE_MOVED:
      // Send an exit to the current |mouse_moved_handler_| and an enter to
      // |target|. Take care that both us and |target| aren't destroyed during
      // dispatch.
      if (target != mouse_moved_handler_) {
        aura::Window* old_mouse_moved_handler = mouse_moved_handler_;
        WindowTracker live_window;
        live_window.Add(target);
        DispatchDetails details =
            DispatchMouseEnterOrExit(target, *event, crui::ET_MOUSE_EXITED);
        // |details| contains information about |mouse_moved_handler_| being
        // destroyed which is not our |target|. Return value of this function
        // should be about our |target|.
        DispatchDetails target_details = details;
        target_details.target_destroyed = !live_window.Contains(target);
        if (details.dispatcher_destroyed) {
          event->SetHandled();
          return target_details;
        }
        // If the |mouse_moved_handler_| changes out from under us, assume a
        // nested run loop ran and we don't need to do anything.
        if (mouse_moved_handler_ != old_mouse_moved_handler) {
          event->SetHandled();
          return target_details;
        }
        if (details.target_destroyed || target_details.target_destroyed) {
          mouse_moved_handler_ = NULL;
          event->SetHandled();
          return target_details;
        }
        live_window.Remove(target);

        mouse_moved_handler_ = target;
        details =
            DispatchMouseEnterOrExit(target, *event, crui::ET_MOUSE_ENTERED);
        if (details.dispatcher_destroyed || details.target_destroyed) {
          event->SetHandled();
          return details;
        }
      }
      break;
    case crui::ET_MOUSE_PRESSED:
      // Don't set the mouse pressed handler for non client mouse down events.
      // These are only sent by Windows and are not always followed with non
      // client mouse up events which causes subsequent mouse events to be
      // sent to the wrong target.
      if (!(event->flags() & crui::EF_IS_NON_CLIENT) && !mouse_pressed_handler_)
        mouse_pressed_handler_ = target;
      break;
    case crui::ET_MOUSE_RELEASED:
      mouse_pressed_handler_ = NULL;
      break;
    default:
      break;
  }

  return PreDispatchLocatedEvent(target, event);
}

DispatchDetails WindowEventDispatcher::PreDispatchPinchEvent(
    Window* target,
    crui::GestureEvent* event) {
  if (event->details().device_type() != crui::GestureDeviceType::DEVICE_TOUCHPAD)
    return PreDispatchLocatedEvent(target, event);
  switch (event->type()) {
    case crui::ET_GESTURE_PINCH_BEGIN:
      touchpad_pinch_handler_ = target;
      break;
    case crui::ET_GESTURE_PINCH_END:
      touchpad_pinch_handler_ = nullptr;
      break;
    default:
      break;
  }

  return PreDispatchLocatedEvent(target, event);
}

DispatchDetails WindowEventDispatcher::PreDispatchTouchEvent(
    Window* target,
    crui::TouchEvent* event) {
  if (event->type() == crui::ET_TOUCH_MOVED && move_hold_count_ &&
      !dispatching_held_event_) {
    held_move_event_ =
        std::make_unique<crui::TouchEvent>(*event, target, window());
    event->SetHandled();
    return DispatchDetails();
  }

  Env::GetInstance()->env_controller()->UpdateStateForTouchEvent(*event);

  crui::TouchEvent root_relative_event(*event);
  root_relative_event.set_location_f(event->root_location_f());
  Env* env = Env::GetInstance();
  if (!env->gesture_recognizer()->ProcessTouchEventPreDispatch(
          &root_relative_event, target)) {
    // The event is invalid - ignore it.
    event->StopPropagation();
    event->DisableSynchronousHandling();
    for (auto& observer : env->window_event_dispatcher_observers())
      observer.OnWindowEventDispatcherIgnoredEvent(this);
    return DispatchDetails();
  }

  // This flag is set depending on the gestures recognized in the call above,
  // and needs to propagate with the forwarded event.
  event->set_may_cause_scrolling(root_relative_event.may_cause_scrolling());

  return PreDispatchLocatedEvent(target, event);
}

DispatchDetails WindowEventDispatcher::PreDispatchKeyEvent(
    crui::KeyEvent* event) {
  if (skip_ime_ || !host_->has_input_method() ||
      (event->flags() & crui::EF_IS_SYNTHESIZED) ||
      !host_->ShouldSendKeyEventToIme()) {
    return DispatchDetails();
  }

  // At this point (i.e: EP_PREDISPATCH), event target is still not set, so do
  // it explicitly here thus making it possible for InputMethodContext
  // implementation to retrieve target window through KeyEvent::target().
  // Event::target is reset at WindowTreeHost::DispatchKeyEventPostIME(), just
  // after key is processed by InputMethodContext.
  ///crui::Event::DispatcherApi(event).set_target(window());
  ///
  ///DispatchDetails details = host_->GetInputMethod()->DispatchKeyEvent(event);
  ///event->StopPropagation();
  ///return details;
  return DispatchDetails();
}

}  // namespace aura
}  // namespace crui
