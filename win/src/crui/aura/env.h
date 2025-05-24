// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_AURA_ENV_H_
#define UI_AURA_ENV_H_

#include <memory>
#include <set>

#include "crbase/functional/callback_forward.h"
#include "crbase/observer_list.h"
#include "crbase/supports_user_data.h"
///#include "mojo/public/cpp/system/buffer.h"
#include "crui/events/event_target.h"
#include "crui/gfx/geometry/point.h"
#include "crui/base/ui_export.h"

namespace crui {

class ContextFactory;
class ContextFactoryPrivate;
class EventObserver;
class GestureRecognizer;
class PlatformEventSource;

namespace aura {

class EnvInputStateController;
class EnvObserver;
class EventObserverAdapter;
class InputStateLookup;
class Window;
class WindowEventDispatcherObserver;
///class WindowOcclusionTracker;
class WindowTreeHost;

// A singleton object that tracks general state within Aura.
class CRUI_EXPORT Env : public crui::EventTarget,
                        public cr::SupportsUserData {
 public:
  Env(const Env&) = delete;
  Env& operator=(const Env&) = delete;

  ~Env() override;

  // Creates a new Env instance.
  static std::unique_ptr<Env> CreateInstance();

  // Returns the instance created by CreateInstance(). This DCHECKs that
  // CreateInstance() has been called. Use HasInstance() to determine if
  // CreateInstance() was called.
  static Env* GetInstance();
  static bool HasInstance();

  void AddObserver(EnvObserver* observer);
  void RemoveObserver(EnvObserver* observer);

  void AddWindowEventDispatcherObserver(
      WindowEventDispatcherObserver* observer);
  void RemoveWindowEventDispatcherObserver(
      WindowEventDispatcherObserver* observer);
  ///cr::ObserverList<WindowEventDispatcherObserver>::Unchecked&
  cr::ObserverList<WindowEventDispatcherObserver>&
    window_event_dispatcher_observers() {
    return window_event_dispatcher_observers_;
  }

  EnvInputStateController* env_controller() const {
    return env_controller_.get();
  }

  int mouse_button_flags() const { return mouse_button_flags_; }
  void set_mouse_button_flags(int mouse_button_flags) {
    mouse_button_flags_ = mouse_button_flags;
  }
  // Returns true if a mouse button is down. This may query the native OS,
  // otherwise it uses |mouse_button_flags_|.
  bool IsMouseButtonDown() const;

  // Gets/sets the last mouse location seen in a mouse event in the screen
  // coordinates.
  const gfx::Point& last_mouse_location() const { return last_mouse_location_; }
  void SetLastMouseLocation(const gfx::Point& last_mouse_location);

  // Whether any touch device is currently down.
  bool is_touch_down() const { return is_touch_down_; }
  void set_touch_down(bool value) { is_touch_down_ = value; }

  void set_context_factory(crui::ContextFactory* context_factory) {
    context_factory_ = context_factory;
  }
  crui::ContextFactory* context_factory() { return context_factory_; }

  // Sets |initial_throttle_input_on_resize| the next time Env is created. This
  // is only useful in tests that need to disable input resize.
  static void set_initial_throttle_input_on_resize_for_testing(
      bool throttle_input) {
    initial_throttle_input_on_resize_ = throttle_input;
  }
  void set_throttle_input_on_resize_for_testing(bool throttle_input) {
    throttle_input_on_resize_ = throttle_input;
  }
  bool throttle_input_on_resize() const { return throttle_input_on_resize_; }

  void set_context_factory_private(
      crui::ContextFactoryPrivate* context_factory_private) {
    context_factory_private_ = context_factory_private;
  }
  crui::ContextFactoryPrivate* context_factory_private() {
    return context_factory_private_;
  }

  crui::GestureRecognizer* gesture_recognizer() {
    return gesture_recognizer_.get();
  }

  void SetGestureRecognizer(
      std::unique_ptr<crui::GestureRecognizer> gesture_recognizer);

  // Get WindowOcclusionTracker instance. Create one if not yet created.
  ///WindowOcclusionTracker* GetWindowOcclusionTracker();

  // Pause/unpause window occlusion tracking.
  void PauseWindowOcclusionTracking();
  void UnpauseWindowOcclusionTracking();

  // Add, remove, or notify EventObservers. EventObservers are essentially
  // pre-target EventHandlers that can not modify the events nor alter dispatch.
  // On Chrome OS, observers receive system-wide events if |target| is this Env.
  // On desktop platforms, observers may only receive events targeting Chrome.
  // Observers must be removed before their target is destroyed.
  void AddEventObserver(crui::EventObserver* observer,
                        crui::EventTarget* target,
                        const std::set<crui::EventType>& types);
  void RemoveEventObserver(crui::EventObserver* observer);
  void NotifyEventObservers(const crui::Event& event);

 private:
  friend class EventInjector;
  friend class Window;
  friend class WindowTreeHost;

  Env();

  void Init();

  // Called by the Window when it is initialized. Notifies observers.
  void NotifyWindowInitialized(Window* window);

  // Called by the WindowTreeHost when it is initialized. Notifies observers.
  void NotifyHostInitialized(WindowTreeHost* host);

  // Overridden from ui::EventTarget:
  bool CanAcceptEvent(const crui::Event& event) override;
  crui::EventTarget* GetParentTarget() override;
  std::unique_ptr<crui::EventTargetIterator> GetChildIterator() const override;
  crui::EventTargeter* GetEventTargeter() override;

  ///cr::ObserverList<EnvObserver>::Unchecked observers_;
  cr::ObserverList<EnvObserver> observers_;

  // Code wanting to observe WindowEventDispatcher typically wants to observe
  // all WindowEventDispatchers. This is made easier by having Env own all the
  // observers.
  ///cr::ObserverList<WindowEventDispatcherObserver>::Unchecked
  cr::ObserverList<WindowEventDispatcherObserver>
    window_event_dispatcher_observers_;

  // The ObserverList and set of owned EventObserver adapters.
  cr::ObserverList<EventObserverAdapter> event_observer_adapter_list_;
  std::set<std::unique_ptr<EventObserverAdapter>> event_observer_adapters_;

  std::unique_ptr<EnvInputStateController> env_controller_;
  int mouse_button_flags_ = 0;
  // Location of last mouse event, in screen coordinates.
  mutable gfx::Point last_mouse_location_;
  bool is_touch_down_ = false;

  std::unique_ptr<crui::GestureRecognizer> gesture_recognizer_;

  std::unique_ptr<InputStateLookup> input_state_lookup_;
  std::unique_ptr<crui::PlatformEventSource> event_source_;

  crui::ContextFactory* context_factory_ = nullptr;
  crui::ContextFactoryPrivate* context_factory_private_ = nullptr;

  static bool initial_throttle_input_on_resize_;
  bool throttle_input_on_resize_ = initial_throttle_input_on_resize_;

  ///std::unique_ptr<WindowOcclusionTracker> window_occlusion_tracker_;
};

}  // namespace aura
}  // namespace crui

#endif  // UI_AURA_ENV_H_
