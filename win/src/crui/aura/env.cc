// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/env.h"

///#include "crbase/command_line.h"
#include "crbase/memory/lazy_instance.h"
#include "crbase/memory/ptr_util.h"
///#include "crbase/observer_list_types.h"
#include "crui/aura/client/aura_constants.h"
#include "crui/aura/env_input_state_controller.h"
#include "crui/aura/env_observer.h"
#include "crui/aura/input_state_lookup.h"
#include "crui/aura/window.h"
#include "crui/aura/window_event_dispatcher_observer.h"
///#include "crui/aura/window_occlusion_tracker.h"
///#include "crui/base/ui_base_features.h"
#include "crui/events/event_observer.h"
#include "crui/events/event_target_iterator.h"
#include "crui/events/event.h"
#include "crui/events/gestures/gesture_recognizer_impl.h"
#include "crui/events/platform/platform_event_source.h"

namespace crui {
namespace aura {

namespace {

// Instance created by all static functions, except
// CreateLocalInstanceForInProcess(). See GetInstance() for details.
Env* g_primary_instance = nullptr;

}  // namespace

// EventObserverAdapter is an aura::Env pre-target handler that forwards
// read-only events to its observer when they match the requested types.
class EventObserverAdapter : public crui::EventHandler/*,
                             public cr::CheckedObserver*/ {
 public:
  EventObserverAdapter(const EventObserverAdapter&) = delete;
  EventObserverAdapter& operator=(const EventObserverAdapter&) = delete;

  EventObserverAdapter(crui::EventObserver* observer,
                       crui::EventTarget* target,
                       const std::set<crui::EventType>& types)
      : observer_(observer), target_(target), types_(types) {
    target_->AddPreTargetHandler(this);
  }

  ~EventObserverAdapter() override { target_->RemovePreTargetHandler(this); }

  crui::EventObserver* observer() { return observer_; }
  crui::EventTarget* target() { return target_; }
  const std::set<crui::EventType>& types() const { return types_; }

  // ui::EventHandler:
  void OnEvent(crui::Event* event) override {
    if (types_.count(event->type()) > 0) {
      std::unique_ptr<crui::Event> cloned_event = crui::Event::Clone(*event);
      crui::Event::DispatcherApi(cloned_event.get()).set_target(event->target());
      // The root location of located events should be in screen coordinates.
      if (cloned_event->IsLocatedEvent() && cloned_event->target()) {
        crui::LocatedEvent* located_event = cloned_event->AsLocatedEvent();
        auto root = located_event->target()->GetScreenLocationF(*located_event);
        located_event->set_root_location_f(root);
      }
      observer_->OnEvent(*cloned_event);
    }
  }

 private:
  crui::EventObserver* observer_;
  crui::EventTarget* target_;
  const std::set<crui::EventType> types_;
};

////////////////////////////////////////////////////////////////////////////////
// Env, public:

Env::~Env() {
  for (EnvObserver& observer : observers_)
    observer.OnWillDestroyEnv();

  if (this == g_primary_instance)
    g_primary_instance = nullptr;
}

// static
std::unique_ptr<Env> Env::CreateInstance() {
  CR_DCHECK(!g_primary_instance);
  // No make_unique as constructor is private.
  std::unique_ptr<Env> env(new Env());
  g_primary_instance = env.get();
  env->Init();
  return env;
}

// static
Env* Env::GetInstance() {
  Env* env = g_primary_instance;
  CR_DCHECK(env) << "Env::CreateInstance must be called before getting the "
                    "instance of Env.";
  return env;
}

// static
bool Env::HasInstance() {
  return !!g_primary_instance;
}

void Env::AddObserver(EnvObserver* observer) {
  observers_.AddObserver(observer);
}

void Env::RemoveObserver(EnvObserver* observer) {
  observers_.RemoveObserver(observer);
}

void Env::AddWindowEventDispatcherObserver(
    WindowEventDispatcherObserver* observer) {
  window_event_dispatcher_observers_.AddObserver(observer);
}

void Env::RemoveWindowEventDispatcherObserver(
    WindowEventDispatcherObserver* observer) {
  window_event_dispatcher_observers_.RemoveObserver(observer);
}

bool Env::IsMouseButtonDown() const {
  return input_state_lookup_.get() ? input_state_lookup_->IsMouseButtonDown() :
      mouse_button_flags_ != 0;
}

void Env::SetLastMouseLocation(const gfx::Point& last_mouse_location) {
  last_mouse_location_ = last_mouse_location;
}

void Env::SetGestureRecognizer(
    std::unique_ptr<crui::GestureRecognizer> gesture_recognizer) {
  gesture_recognizer_ = std::move(gesture_recognizer);
}

///WindowOcclusionTracker* Env::GetWindowOcclusionTracker() {
///  if (!window_occlusion_tracker_) {
///    // Use cr::WrapUnique + new because of the constructor is private.
///    window_occlusion_tracker_ = cr::WrapUnique(new WindowOcclusionTracker());
///  }
///
///  return window_occlusion_tracker_.get();
///}

void Env::PauseWindowOcclusionTracking() {
  ///const bool was_paused = !!GetWindowOcclusionTracker();
  ///GetWindowOcclusionTracker()->Pause();
  ///if (!was_paused) {
  ///  for (EnvObserver& observer : observers_)
  ///    observer.OnWindowOcclusionTrackingPaused();
  ///}
}

void Env::UnpauseWindowOcclusionTracking() {
  ///GetWindowOcclusionTracker()->Unpause();
  ///if (!GetWindowOcclusionTracker()->IsPaused()) {
  ///  for (EnvObserver& observer : observers_)
  ///    observer.OnWindowOcclusionTrackingResumed();
  ///}
}

void Env::AddEventObserver(crui::EventObserver* observer,
                           crui::EventTarget* target,
                           const std::set<crui::EventType>& types) {
  CR_DCHECK(!types.empty()) << "Observers must observe at least one event type";
  auto adapter(std::make_unique<EventObserverAdapter>(observer, target, types));
  event_observer_adapter_list_.AddObserver(adapter.get());
  event_observer_adapters_.insert(std::move(adapter));
}

void Env::RemoveEventObserver(crui::EventObserver* observer) {
  for (auto& adapter : event_observer_adapters_) {
    if (adapter->observer() == observer) {
      event_observer_adapter_list_.RemoveObserver(adapter.get());
      event_observer_adapters_.erase(adapter);
      return;
    }
  }
}

void Env::NotifyEventObservers(const crui::Event& event) {
  for (auto& adapter : event_observer_adapter_list_) {
    if (adapter.types().count(event.type()) > 0 &&
        (adapter.target() == event.target() || adapter.target() == this)) {
      adapter.observer()->OnEvent(event);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Env, private:

// static
bool Env::initial_throttle_input_on_resize_ = true;

Env::Env()
    : env_controller_(std::make_unique<EnvInputStateController>(this)),
      gesture_recognizer_(std::make_unique<crui::GestureRecognizerImpl>()),
      input_state_lookup_(InputStateLookup::Create()) {}

void Env::Init() {
  if (!crui::PlatformEventSource::GetInstance())
    event_source_ = crui::PlatformEventSource::CreateDefault();
}

void Env::NotifyWindowInitialized(Window* window) {
  for (EnvObserver& observer : observers_)
    observer.OnWindowInitialized(window);
}

void Env::NotifyHostInitialized(WindowTreeHost* host) {
  for (EnvObserver& observer : observers_)
    observer.OnHostInitialized(host);
}

////////////////////////////////////////////////////////////////////////////////
// Env, ui::EventTarget implementation:

bool Env::CanAcceptEvent(const crui::Event& event) {
  return true;
}

crui::EventTarget* Env::GetParentTarget() {
  return nullptr;
}

std::unique_ptr<crui::EventTargetIterator> Env::GetChildIterator() const {
  return nullptr;
}

crui::EventTargeter* Env::GetEventTargeter() {
  CR_NOTREACHED();
  return nullptr;
}

}  // namespace aura
}  // namespace crui
