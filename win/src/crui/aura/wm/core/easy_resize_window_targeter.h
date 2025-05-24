// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_WM_CORE_EASY_RESIZE_WINDOW_TARGETER_H_
#define UI_WM_CORE_EASY_RESIZE_WINDOW_TARGETER_H_

#include "crui/aura/window_targeter.h"
#include "crui/base/ui_export.h"

namespace crui {

namespace gfx {
class Insets;
}  // namespace gfx

namespace wm {

// An EventTargeter for a container window that uses a slightly larger
// hit-target region for easier resize. It extends the hit test region for child
// windows (top level Widgets that are resizable) to outside their bounds. For
// Ash, this correlates to ash::kResizeOutsideBoundsSize. For the interior
// resize area, see ash::InstallResizeHandleWindowTargeterForWindow().
class CRUI_EXPORT EasyResizeWindowTargeter : public aura::WindowTargeter {
 public:
  EasyResizeWindowTargeter(const EasyResizeWindowTargeter&) = delete;
  EasyResizeWindowTargeter& operator=(
      const EasyResizeWindowTargeter&) = delete;

  // NOTE: the insets must be negative.
  EasyResizeWindowTargeter(const gfx::Insets& mouse_extend,
                           const gfx::Insets& touch_extend);

  ~EasyResizeWindowTargeter() override;

 private:
  // aura::WindowTargeter:
  // Delegates to WindowTargeter's impl and prevents overriding in subclasses.
  bool EventLocationInsideBounds(aura::Window* target,
                                 const crui::LocatedEvent& event) const override;

  // Returns true if the hit testing (GetHitTestRects()) should use the
  // extended bounds.
  bool ShouldUseExtendedBounds(const aura::Window* w) const override;
};

}  // namespace wm
}  // namespace crui

#endif  // UI_WM_CORE_EASY_RESIZE_WINDOW_TARGETER_H_
