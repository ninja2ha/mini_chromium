// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_WIDGET_TOOLTIP_MANAGER_AURA_H_
#define UI_VIEWS_WIDGET_TOOLTIP_MANAGER_AURA_H_

#include "crbase/compiler_specific.h"
#include "crbase/strings/string16.h"
#include "crui/gfx/geometry/point.h"
#include "crui/views/widget/tooltip_manager.h"
#include "crui/base/ui_export.h"

namespace crui {

namespace aura {
class Window;
}  // namespace aura

namespace gfx {
class FontList;
}  // namespace gfx

namespace views {

class Widget;

// TooltipManager implementation for Aura.
class CRUI_EXPORT TooltipManagerAura : public TooltipManager {
 public:
  TooltipManagerAura(const TooltipManagerAura&) = delete;
  TooltipManagerAura& operator=(const TooltipManagerAura&) = delete;

  explicit TooltipManagerAura(Widget* widget);
  ~TooltipManagerAura() override;

  // If |source| has capture this finds the Widget under the mouse and invokes
  // UpdateTooltip() on it's TooltipManager. This is necessary as when capture
  // is held mouse events are only delivered to the Window that has capture even
  // though we may show tooltips for the Window under the mouse.
  static void UpdateTooltipManagerForCapture(Widget* source);

  // Returns the FontList used by all TooltipManagerAuras.
  ///static const gfx::FontList& GetDefaultFontList();

  // TooltipManager:
  int GetMaxWidth(const gfx::Point& location) const override;
  ///const gfx::FontList& GetFontList() const override;
  void UpdateTooltip() override;
  void TooltipTextChanged(View* view) override;

 private:
  View* GetViewUnderPoint(const gfx::Point& point);
  void UpdateTooltipForTarget(View* target,
                              const gfx::Point& point,
                              aura::Window* root_window);

  // Returns the Window the tooltip text is installed on.
  aura::Window* GetWindow();

  Widget* widget_;
  cr::string16 tooltip_text_;
};

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_WIDGET_TOOLTIP_MANAGER_AURA_H_
