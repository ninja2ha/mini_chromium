// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_WM_CORE_WINDOW_MODALITY_CONTROLLER_H_
#define UI_WM_CORE_WINDOW_MODALITY_CONTROLLER_H_

#include <vector>

#include "crbase/compiler_specific.h"
#include "crui/aura/env_observer.h"
#include "crui/aura/window_observer.h"
#include "crui/events/event_handler.h"
#include "crui/base/ui_export.h"

namespace crui {

class EventTarget;
class LocatedEvent;

namespace aura {
class Env;
}  // namespace aura

namespace wm {

// Sets the modal parent for the child.
CRUI_EXPORT void SetModalParent(aura::Window* child, aura::Window* parent);

// Returns the modal transient child of |window|, or NULL if |window| does not
// have any modal transient children.
CRUI_EXPORT aura::Window* GetModalTransient(aura::Window* window);
CRUI_EXPORT const aura::Window* GetModalTransient(
    const aura::Window* window);

// WindowModalityController is an event filter that consumes events sent to
// windows that are the transient parents of window-modal windows. This filter
// must be added to the CompoundEventFilter so that activation works properly.
class CRUI_EXPORT WindowModalityController : public crui::EventHandler,
                                             public aura::EnvObserver,
                                             public aura::WindowObserver {
 public:
  WindowModalityController(const WindowModalityController&) = delete;
  WindowModalityController& operator=(const WindowModalityController&) = delete;

  explicit WindowModalityController(crui::EventTarget* event_target,
                                    aura::Env* env = nullptr);
  ~WindowModalityController() override;

  // Overridden from ui::EventHandler:
  void OnKeyEvent(crui::KeyEvent* event) override;
  void OnMouseEvent(crui::MouseEvent* event) override;
  void OnTouchEvent(crui::TouchEvent* event) override;

  // Overridden from aura::EnvObserver:
  void OnWindowInitialized(aura::Window* window) override;

  // Overridden from aura::WindowObserver:
  void OnWindowPropertyChanged(aura::Window* window,
                               const void* key,
                               intptr_t old) override;
  void OnWindowVisibilityChanged(aura::Window* window, bool visible) override;
  void OnWindowDestroyed(aura::Window* window) override;

 private:
  // Processes a mouse/touch event, and returns true if the event should be
  // consumed.
  bool ProcessLocatedEvent(aura::Window* target, crui::LocatedEvent* event);

  // Cancel touches on the transient window tree rooted to the top level
  // transient window of the |window|.
  void CancelTouchesOnTransientWindowTree(aura::Window* window);

  aura::Env* env_;

  std::vector<aura::Window*> windows_;

  crui::EventTarget* event_target_;
};

}  // namespace wm
}  // namespace crui

#endif  // UI_WM_CORE_WINDOW_MODALITY_CONTROLLER_H_
