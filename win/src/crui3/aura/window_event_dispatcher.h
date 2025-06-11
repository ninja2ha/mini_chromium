// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_AURA_WINDOW_EVENT_DISPATCHER_H_
#define UI_AURA_WINDOW_EVENT_DISPATCHER_H_

#include <memory>
#include <queue>
#include <vector>

#include "crbase/functional/callback.h"
#include "crbase/memory/ref_counted.h"
#include "crbase/memory/weak_ptr.h"
#include "crbase/scoped_observer.h"
#include "crui/base/ui_export.h"
#include "crui/aura/client/capture_delegate.h"
#include "crui/aura/env_observer.h"
#include "crui/aura/window.h"
#include "crui/aura/window_observer.h"
#include "crui/base/cursor/cursor.h"
#include "crui/events/event_constants.h"
#include "crui/events/event_processor.h"
#include "crui/events/event_targeter.h"
///#include "crui/events/fraction_of_time_without_user_input_recorder.h"
#include "crui/events/gestures/gesture_recognizer.h"
#include "crui/events/gestures/gesture_types.h"
#include "crui/gfx/geometry/point.h"
#include "crui/gfx/native_widget_types.h"

namespace crui {

class Event;
class GestureEvent;
class GestureRecognizer;
class MouseEvent;
class TouchEvent;

namespace aura {
class WindowTargeter;
class WindowTreeHost;

// WindowEventDispatcher orchestrates event dispatch within a window tree
// owned by WindowTreeHost. WTH also owns the WED.
// TODO(beng): In progress, remove functionality not directly related to
//             event dispatch.
class CRUI_EXPORT WindowEventDispatcher : public crui::EventProcessor,
                                          public crui::GestureEventHelper,
                                          public client::CaptureDelegate,
                                          public WindowObserver,
                                          public EnvObserver {
 public:
  WindowEventDispatcher(const WindowEventDispatcher&) = delete;
  WindowEventDispatcher& operator=(const WindowEventDispatcher&) = delete;

  explicit WindowEventDispatcher(WindowTreeHost* host);
  ~WindowEventDispatcher() override;

  // Stops dispatching/synthesizing mouse events.
  void Shutdown();

  WindowTreeHost* host() { return host_; }

  Window* mouse_pressed_handler() { return mouse_pressed_handler_; }
  Window* mouse_moved_handler() { return mouse_moved_handler_; }
  Window* touchpad_pinch_handler() { return touchpad_pinch_handler_; }

  WindowTargeter* event_targeter() { return event_targeter_.get(); }

  // Overridden from ui::EventProcessor:
  crui::EventTargeter* GetDefaultEventTargeter() override;

  // Repost event for re-processing. Used when exiting context menus.
  // We support the ET_MOUSE_PRESSED, ET_TOUCH_PRESSED and ET_GESTURE_TAP_DOWN
  // event types (although the latter is currently a no-op).
  void RepostEvent(const crui::LocatedEvent* event);

  // Invoked when the mouse events get enabled or disabled.
  void OnMouseEventsEnableStateChanged(bool enabled);

  void DispatchCancelModeEvent();

  // Dispatches a ui::ET_MOUSE_EXITED event at |point| to the |target|
  // If the |target| is NULL, we will dispatch the event to the root-window.
  // |event_flags| will be set on the dispatched exit event.
  // TODO(beng): needed only for WTH::OnCursorVisibilityChanged().
  crui::EventDispatchDetails DispatchMouseExitAtPoint(
      Window* target,
      const gfx::Point& point,
      int event_flags = crui::EF_NONE) CR_WARN_UNUSED_RESULT;

  // Gesture Recognition -------------------------------------------------------

  // When a touch event is dispatched to a Window, it may want to process the
  // touch event asynchronously. In such cases, the window should consume the
  // event during the event dispatch. Once the event is properly processed, the
  // window should let the WindowEventDispatcher know about the result of the
  // event processing, so that gesture events can be properly created and
  // dispatched. |event|'s location should be in the dispatcher's coordinate
  // space, in DIPs.
  virtual void ProcessedTouchEvent(uint32_t unique_event_id,
                                   Window* window,
                                   crui::EventResult result,
                                   bool is_source_touch_event_set_non_blocking);

  // These methods are used to defer the processing of mouse/touch events
  // related to resize. A client (typically a RenderWidgetHostViewAura) can call
  // HoldPointerMoves when an resize is initiated and then ReleasePointerMoves
  // once the resize is completed.
  //
  // More than one hold can be invoked and each hold must be cancelled by a
  // release before we resume normal operation.
  void HoldPointerMoves();
  void ReleasePointerMoves();

  // Gets the last location seen in a mouse event in this root window's
  // coordinates. This may return a point outside the root window's bounds.
  gfx::Point GetLastMouseLocationInRoot() const;

  void OnHostLostMouseGrab();
  void OnCursorMovedToRootLocation(const gfx::Point& root_location);

  // TODO(beng): This is only needed because this cleanup needs to happen after
  //             all other observers are notified of OnWindowDestroying() but
  //             before OnWindowDestroyed() is sent (i.e. while the window
  //             hierarchy is still intact). This didn't seem worth adding a
  //             generic notification for as only this class needs to implement
  //             it. I would however like to find a way to do this via an
  //             observer.
  void OnPostNotifiedWindowDestroying(Window* window);

  // True to skip sending event to the InputMethod.
  void set_skip_ime(bool skip_ime) { skip_ime_ = skip_ime; }
  bool should_skip_ime() const { return skip_ime_; }

 private:
  friend class Window;

  // Used to call WindowEventDispatcherObserver when event processing starts
  // (from the constructor) and finishes (from the destructor). Notification is
  // handled by this object to ensure notification happens if the associated
  // WindowEventDispatcher is destroyed during processing of the event.
  class ObserverNotifier {
   public:
    ObserverNotifier(const ObserverNotifier&) = delete;
    ObserverNotifier& operator=(const ObserverNotifier&) = delete;

    ObserverNotifier(WindowEventDispatcher* dispatcher, const crui::Event& event);
    ~ObserverNotifier();

   private:
    WindowEventDispatcher* dispatcher_;
  };

  // The parameter for OnWindowHidden() to specify why window is hidden.
  enum WindowHiddenReason {
    WINDOW_DESTROYED,  // Window is destroyed.
    WINDOW_HIDDEN,     // Window is hidden.
    WINDOW_MOVING,     // Window is temporarily marked as hidden due to move
                       // across root windows.
  };

  Window* window();
  const Window* window() const;

  // Converts a point from screen coordinates to the coordinate space used by
  // the Window returned from window().
  void ConvertPointFromScreen(gfx::Point* screen_point) const;

  // Updates the event with the appropriate transform for the device scale
  // factor. The WindowEventDispatcher dispatches events in the physical pixel
  // coordinate. But the event processing from WindowEventDispatcher onwards
  // happen in device-independent pixel coordinate. So it is necessary to update
  // the event received from the host.
  void TransformEventForDeviceScaleFactor(crui::LocatedEvent* event);

  // Dispatches OnMouseExited to the |window| which is hiding if necessary.
  void DispatchMouseExitToHidingWindow(Window* window);

  // Dispatches the specified event type (intended for enter/exit) to the
  // |mouse_moved_handler_|.
  // The event's location will be converted from |target|coordinate system to
  // |mouse_moved_handler_| coordinate system.
  crui::EventDispatchDetails DispatchMouseEnterOrExit(
      Window* target,
      const crui::MouseEvent& event,
      crui::EventType type) CR_WARN_UNUSED_RESULT;
  crui::EventDispatchDetails ProcessGestures(
      Window* target,
      crui::GestureRecognizer::Gestures gestures) CR_WARN_UNUSED_RESULT;

  // Called when a window becomes invisible, either by being removed
  // from root window hierarchy, via SetVisible(false) or being destroyed.
  // |reason| specifies what triggered the hiding. Note that becoming invisible
  // will cause a window to lose capture and some windows may destroy themselves
  // on capture (like DragDropTracker).
  void OnWindowHidden(Window* invisible, WindowHiddenReason reason);

  bool is_dispatched_held_event(const crui::Event& event) const;

  // Overridden from aura::client::CaptureDelegate:
  void UpdateCapture(Window* old_capture, Window* new_capture) override;
  void OnOtherRootGotCapture() override;
  void SetNativeCapture() override;
  void ReleaseNativeCapture() override;

  // Overridden from ui::EventProcessor:
  crui::EventTarget* GetRootForEvent(crui::Event* event) override;
  void OnEventProcessingStarted(crui::Event* event) override;
  void OnEventProcessingFinished(crui::Event* event) override;

  // Overridden from ui::EventDispatcherDelegate.
  bool CanDispatchToTarget(crui::EventTarget* target) override;
  crui::EventDispatchDetails PreDispatchEvent(crui::EventTarget* target,
                                               crui::Event* event) override;
  crui::EventDispatchDetails PostDispatchEvent(
      crui::EventTarget* target, const crui::Event& event) override;

  // Overridden from ui::GestureEventHelper.
  bool CanDispatchToConsumer(crui::GestureConsumer* consumer) override;
  void DispatchGestureEvent(crui::GestureConsumer* raw_input_consumer,
                            crui::GestureEvent* event) override;
  void DispatchSyntheticTouchEvent(crui::TouchEvent* event) override;

  // Overridden from WindowObserver:
  void OnWindowDestroying(Window* window) override;
  void OnWindowDestroyed(Window* window) override;
  void OnWindowAddedToRootWindow(Window* window) override;
  void OnWindowRemovingFromRootWindow(Window* window,
                                      Window* new_root) override;
  void OnWindowVisibilityChanging(Window* window, bool visible) override;
  void OnWindowVisibilityChanged(Window* window, bool visible) override;
  void OnWindowBoundsChanged(Window* window,
                             const gfx::Rect& old_bounds,
                             const gfx::Rect& new_bounds,
                             crui::PropertyChangeReason reason) override;
  void OnWindowTargetTransformChanging(
      Window* window,
      const gfx::Transform& new_transform) override;
  void OnWindowTransformed(Window* window,
                           crui::PropertyChangeReason reason) override;

  // Overridden from EnvObserver:
  void OnWindowInitialized(Window* window) override;

  // We hold and aggregate mouse drags and touch moves as a way of throttling
  // resizes when HoldMouseMoves() is called. The following methods are used to
  // dispatch held and newly incoming mouse and touch events, typically when an
  // event other than one of these needs dispatching or a matching
  // ReleaseMouseMoves()/ReleaseTouchMoves() is called.  NOTE: because these
  // methods dispatch events from WindowTreeHost the coordinates are in terms of
  // the root.
  crui::EventDispatchDetails DispatchHeldEvents() CR_WARN_UNUSED_RESULT;

  // Posts a task to send synthesized mouse move event if there is no a pending
  // task.
  void PostSynthesizeMouseMove();

  // Creates and dispatches synthesized mouse move event using the current mouse
  // location.
  crui::EventDispatchDetails SynthesizeMouseMoveEvent() CR_WARN_UNUSED_RESULT;

  // Calls SynthesizeMouseMove() if |window| is currently visible and contains
  // the mouse cursor.
  void SynthesizeMouseMoveAfterChangeToWindow(Window* window);

  crui::EventDispatchDetails PreDispatchLocatedEvent(Window* target,
                                                     crui::LocatedEvent* event);
  crui::EventDispatchDetails PreDispatchMouseEvent(Window* target,
                                                   crui::MouseEvent* event);
  crui::EventDispatchDetails PreDispatchPinchEvent(Window* target,
                                                   crui::GestureEvent* event);
  crui::EventDispatchDetails PreDispatchTouchEvent(Window* target,
                                                   crui::TouchEvent* event);
  crui::EventDispatchDetails PreDispatchKeyEvent(crui::KeyEvent* event);

  WindowTreeHost* host_;

  Window* mouse_pressed_handler_ = nullptr;
  Window* mouse_moved_handler_ = nullptr;
  Window* touchpad_pinch_handler_ = nullptr;
  Window* event_dispatch_target_ = nullptr;
  Window* old_dispatch_target_ = nullptr;

  ///crui::FractionOfTimeWithoutUserInputRecorder
  ///    fraction_of_time_without_user_input_recorder_;

  bool synthesize_mouse_move_ = false;

  // Whether a OnWindowTargetTransformChanging() call didn't have its
  // corresponding OnWindowTransformed() call yet.
  bool window_transforming_ = false;

  // How many move holds are outstanding. We try to defer dispatching
  // touch/mouse moves while the count is > 0.
  int move_hold_count_ = 0;
  // The location of |held_move_event_| is in |window_|'s coordinate.
  std::unique_ptr<crui::LocatedEvent> held_move_event_;

  // Allowing for reposting of events. Used when exiting context menus.
  std::unique_ptr<crui::LocatedEvent> held_repostable_event_;

  // Set when dispatching a held event.
  crui::LocatedEvent* dispatching_held_event_ = nullptr;

  cr::ScopedObserver<aura::Window, aura::WindowObserver> observer_manager_;

  // The default EventTargeter for WindowEventDispatcher generated events.
  std::unique_ptr<WindowTargeter> event_targeter_;

  bool skip_ime_ = false;

  // This callback is called when the held move event is dispatched, or when
  // pointer moves are released and there is no held move event.
  cr::OnceClosure did_dispatch_held_move_event_callback_;

  // See ObserverNotifier for details. This is a queue to handle the case of
  // nested event dispatch.
  std::queue<std::unique_ptr<ObserverNotifier>> observer_notifiers_;

  bool in_shutdown_ = false;

  // Used to schedule reposting an event.
  cr::WeakPtrFactory<WindowEventDispatcher> repost_event_factory_{this};

  // Used to schedule DispatchHeldEvents() when |move_hold_count_| goes to 0.
  cr::WeakPtrFactory<WindowEventDispatcher> held_event_factory_{this};
};

}  // namespace aura
}  // namespace crui

#endif  // UI_AURA_WINDOW_EVENT_DISPATCHER_H_
