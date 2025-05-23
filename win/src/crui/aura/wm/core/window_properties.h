// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_WM_CORE_WINDOW_PROPERTIES_H_
#define UI_WM_CORE_WINDOW_PROPERTIES_H_

#include "crui/base/class_property.h"
#include "crui/base/ui_export.h"

namespace crui {
namespace wm {

// Type of visibility change transition that a window should animate.
// Default behavior is to animate both show and hide.
enum WindowVisibilityAnimationTransition {
  ANIMATE_SHOW = 0x1,
  ANIMATE_HIDE = 0x2,
  ANIMATE_BOTH = ANIMATE_SHOW | ANIMATE_HIDE,
  ANIMATE_NONE = 0x4,
};

// Alphabetical sort.

// Property to tell if the container uses screen coordinates for the child
// windows.
CRUI_EXPORT extern const crui::ClassProperty<bool>* const
    kUsesScreenCoordinatesKey;

CRUI_EXPORT extern const crui::ClassProperty<cr::TimeDelta>* const
    kWindowVisibilityAnimationDurationKey;

CRUI_EXPORT extern const crui::ClassProperty<
    WindowVisibilityAnimationTransition>* const
    kWindowVisibilityAnimationTransitionKey;

CRUI_EXPORT extern const crui::ClassProperty<int>* const
    kWindowVisibilityAnimationTypeKey;

// Used if the animation-type is WINDOW_VISIBILITY_ANIMATION_TYPE_VERTICAL.
CRUI_EXPORT extern const crui::ClassProperty<float>* const
    kWindowVisibilityAnimationVerticalPositionKey;

}  // namespace wm
}  // namespace crui

// These need to be declared here for jumbo builds.
DECLARE_EXPORTED_UI_CLASS_PROPERTY_TYPE(CRUI_EXPORT,
                                        wm::WindowVisibilityAnimationTransition)
DECLARE_EXPORTED_UI_CLASS_PROPERTY_TYPE(CRUI_EXPORT, float)

#endif  // UI_WM_CORE_WINDOW_PROPERTIES_H_
