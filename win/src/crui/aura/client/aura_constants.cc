// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/client/aura_constants.h"

#include "crui/base/class_property.h"
#include "crui/gfx/geometry/rect.h"

DEFINE_EXPORTED_UI_CLASS_PROPERTY_TYPE(CRUI_EXPORT, bool)
DEFINE_EXPORTED_UI_CLASS_PROPERTY_TYPE(CRUI_EXPORT, cr::TimeDelta)
///DEFINE_EXPORTED_UI_CLASS_PROPERTY_TYPE(CRUI_EXPORT, cr::UnguessableToken*)
DEFINE_EXPORTED_UI_CLASS_PROPERTY_TYPE(CRUI_EXPORT, cr::string16*)
DEFINE_EXPORTED_UI_CLASS_PROPERTY_TYPE(CRUI_EXPORT, crui::ModalType)
DEFINE_EXPORTED_UI_CLASS_PROPERTY_TYPE(CRUI_EXPORT, crui::ZOrderLevel)
///DEFINE_EXPORTED_UI_CLASS_PROPERTY_TYPE(CRUI_EXPORT, gfx::ImageSkia*)
DEFINE_EXPORTED_UI_CLASS_PROPERTY_TYPE(CRUI_EXPORT, gfx::NativeViewAccessible)
DEFINE_EXPORTED_UI_CLASS_PROPERTY_TYPE(CRUI_EXPORT, gfx::Rect*)
DEFINE_EXPORTED_UI_CLASS_PROPERTY_TYPE(CRUI_EXPORT, gfx::Size*)
DEFINE_EXPORTED_UI_CLASS_PROPERTY_TYPE(CRUI_EXPORT, gfx::SizeF*)
DEFINE_EXPORTED_UI_CLASS_PROPERTY_TYPE(CRUI_EXPORT, std::string*)
DEFINE_EXPORTED_UI_CLASS_PROPERTY_TYPE(CRUI_EXPORT, crui::WindowShowState)
DEFINE_EXPORTED_UI_CLASS_PROPERTY_TYPE(CRUI_EXPORT, void*)
///DEFINE_EXPORTED_UI_CLASS_PROPERTY_TYPE(CRUI_EXPORT, gfx::Color)
DEFINE_EXPORTED_UI_CLASS_PROPERTY_TYPE(CRUI_EXPORT, int32_t)
DEFINE_EXPORTED_UI_CLASS_PROPERTY_TYPE(CRUI_EXPORT, int64_t)
DEFINE_EXPORTED_UI_CLASS_PROPERTY_TYPE(CRUI_EXPORT, aura::client::FocusClient*)
DEFINE_EXPORTED_UI_CLASS_PROPERTY_TYPE(CRUI_EXPORT, aura::Window*)
DEFINE_EXPORTED_UI_CLASS_PROPERTY_TYPE(CRUI_EXPORT, std::vector<aura::Window*>*)

namespace crui {

namespace aura {
namespace client {

// Alphabetical sort.

DEFINE_UI_CLASS_PROPERTY_KEY(bool,
                             kAccessibilityFocusFallsbackToWidgetKey,
                             true)
DEFINE_UI_CLASS_PROPERTY_KEY(bool,
                             kAccessibilityTouchExplorationPassThrough,
                             false)
DEFINE_UI_CLASS_PROPERTY_KEY(bool, kActivateOnPointerKey, true)
DEFINE_UI_CLASS_PROPERTY_KEY(bool, kAnimationsDisabledKey, false)
///DEFINE_OWNED_UI_CLASS_PROPERTY_KEY(gfx::ImageSkia, kAppIconKey, nullptr)
DEFINE_UI_CLASS_PROPERTY_KEY(int, kAppType, 0)
DEFINE_OWNED_UI_CLASS_PROPERTY_KEY(gfx::SizeF, kAspectRatio, nullptr)
///DEFINE_OWNED_UI_CLASS_PROPERTY_KEY(gfx::ImageSkia, kAvatarIconKey, nullptr)
DEFINE_UI_CLASS_PROPERTY_KEY(bool, kWindowLayerDrawn, false)
DEFINE_UI_CLASS_PROPERTY_KEY(bool, kConstrainedWindowKey, false)
DEFINE_UI_CLASS_PROPERTY_KEY(bool, kCreatedByUserGesture, false)
DEFINE_UI_CLASS_PROPERTY_KEY(bool, kDrawAttentionKey, false)
DEFINE_OWNED_UI_CLASS_PROPERTY_KEY(gfx::Rect,
                                   kEmbeddedWindowEnsureNotInRect,
                                   nullptr)
DEFINE_UI_CLASS_PROPERTY_KEY(FocusClient*, kFocusClientKey, nullptr)
DEFINE_UI_CLASS_PROPERTY_KEY(Window*, kHostWindowKey, nullptr)
DEFINE_UI_CLASS_PROPERTY_KEY(Window*, kChildModalParentKey, nullptr)
DEFINE_UI_CLASS_PROPERTY_KEY(crui::ModalType, kModalKey, crui::MODAL_TYPE_NONE)
DEFINE_OWNED_UI_CLASS_PROPERTY_KEY(std::string, kNameKey, nullptr)
DEFINE_UI_CLASS_PROPERTY_KEY(gfx::NativeViewAccessible,
                             kParentNativeViewAccessibleKey,
                             nullptr)
DEFINE_OWNED_UI_CLASS_PROPERTY_KEY(gfx::Size, kPreferredSize, nullptr)
DEFINE_UI_CLASS_PROPERTY_KEY(crui::WindowShowState,
                             kPreMinimizedShowStateKey,
                             crui::SHOW_STATE_DEFAULT)
DEFINE_UI_CLASS_PROPERTY_KEY(crui::WindowShowState,
                             kPreFullscreenShowStateKey,
                             crui::SHOW_STATE_DEFAULT)
DEFINE_UI_CLASS_PROPERTY_KEY(int, kResizeBehaviorKey, kResizeBehaviorCanResize)
DEFINE_OWNED_UI_CLASS_PROPERTY_KEY(gfx::Rect, kRestoreBoundsKey, nullptr)
DEFINE_UI_CLASS_PROPERTY_KEY(crui::WindowShowState,
                             kShowStateKey,
                             crui::SHOW_STATE_DEFAULT)
DEFINE_OWNED_UI_CLASS_PROPERTY_KEY(cr::string16, kTitleKey, nullptr)
DEFINE_UI_CLASS_PROPERTY_KEY(int, kTopViewInset, 0)
///DEFINE_OWNED_UI_CLASS_PROPERTY_KEY(gfx::ImageSkia, kWindowIconKey, nullptr)
DEFINE_UI_CLASS_PROPERTY_KEY(int, kWindowCornerRadiusKey, -1)
DEFINE_UI_CLASS_PROPERTY_KEY(crui::ZOrderLevel,
                             kZOrderingKey,
                             crui::ZOrderLevel::kNormal)

}  // namespace client
}  // namespace aura
}  // namespace crui
