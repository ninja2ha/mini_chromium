// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_WM_CORE_VISIBILITY_CONTROLLER_H_
#define UI_WM_CORE_VISIBILITY_CONTROLLER_H_

#include "crbase/compiler_specific.h"
#include "crbase/logging.h"
#include "crui/aura/client/visibility_client.h"
#include "crui/base/ui_export.h"

namespace crui {
namespace wm {

class CRUI_EXPORT VisibilityController
    : public aura::client::VisibilityClient {
 public:
  VisibilityController(const VisibilityController&) = delete;
  VisibilityController& operator=(const VisibilityController&) = delete;

  VisibilityController();
  ~VisibilityController() override;

 protected:
  // Subclasses override if they want to call a different implementation of
  // this function.
  // TODO(beng): potentially replace by an actual window animator class in
  //             window_animations.h.
  virtual bool CallAnimateOnChildWindowVisibilityChanged(aura::Window* window,
                                                         bool visible);

 private:
  // Overridden from aura::client::VisibilityClient:
  void UpdateLayerVisibility(aura::Window* window, bool visible) override;
};

// Suspends the animations for visibility changes during the lifetime of an
// instance of this class.
//
// Example:
//
// void ViewName::UnanimatedAction() {
//   SuspendChildWindowVisibilityAnimations suspend(parent);
//   // Perform unanimated action here.
//   // ...
//   // When the method finishes, visibility animations will return to their
//   // previous state.
// }
//
class CRUI_EXPORT SuspendChildWindowVisibilityAnimations {
 public:
  SuspendChildWindowVisibilityAnimations(
      const SuspendChildWindowVisibilityAnimations&) = delete;
  SuspendChildWindowVisibilityAnimations& operator=(
      const SuspendChildWindowVisibilityAnimations&) = delete;
  // Suspend visibility animations of child windows.
  explicit SuspendChildWindowVisibilityAnimations(aura::Window* window);

  // Restore visibility animations to their original state.
  ~SuspendChildWindowVisibilityAnimations();

 private:
  // The window to manage.
  aura::Window* window_;

  // Whether the visibility animations on child windows were originally enabled.
  const bool original_enabled_;
};

// Enable visibility change animation for specific |window|. Use this if
// you want to enable visibility change animation on a specific window without
// affecting other windows in the same container. Calling this on a window
// whose animation is already enabled either by this function, or
// via SetChildWindowVisibilityChangesAnimatedbelow below is allowed and
// the animation stays enabled.
void CRUI_EXPORT SetWindowVisibilityChangesAnimated(aura::Window* window);

// Enable visibility change animation for all children of the |window|.
// Typically applied to a container whose child windows should be animated
// when their visibility changes.
void CRUI_EXPORT
SetChildWindowVisibilityChangesAnimated(aura::Window* window);

}  // namespace wm
}  // namespace crui

#endif  // UI_WM_CORE_VISIBILITY_CONTROLLER_H_
