// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_WM_PUBLIC_TOOLTIP_CLIENT_H_
#define UI_WM_PUBLIC_TOOLTIP_CLIENT_H_

#include "crui/aura/window.h"
///#include "crui/gfx/font.h"
#include "crui/base/ui_export.h"

namespace crui {

namespace gfx {
class Point;
}  // namespace gfx

namespace wm {

class ScopedTooltipDisabler;

class CRUI_EXPORT TooltipClient {
 public:
  // Returns the max width of the tooltip when shown at the specified location.
  virtual int GetMaxWidth(const gfx::Point& point) const = 0;

  // Informs the shell tooltip manager of change in tooltip for window |target|.
  virtual void UpdateTooltip(aura::Window* target) = 0;

  // Sets the time after which the tooltip is hidden for Window |target|. If
  // |timeout_in_ms| is <= 0, the tooltip is shown indefinitely.
  virtual void SetTooltipShownTimeout(aura::Window* target,
                                      int timeout_in_ms) = 0;

 protected:
  // Enables/Disables tooltips. This is treated as a reference count. Consumers
  // must use ScopedTooltipDisabler to enable/disabled tooltips.
  virtual void SetTooltipsEnabled(bool enable) = 0;

 private:
  friend class ScopedTooltipDisabler;
};

CRUI_EXPORT void SetTooltipClient(aura::Window* root_window,
                                  TooltipClient* client);
CRUI_EXPORT TooltipClient* GetTooltipClient(aura::Window* root_window);

// Sets the text for the tooltip. The id is used to determine uniqueness when
// the text does not change. For example, if the tooltip text does not change,
// but the id does then the position of the tooltip is updated.
CRUI_EXPORT void SetTooltipText(aura::Window* window,
                                cr::string16* tooltip_text);
CRUI_EXPORT void SetTooltipId(aura::Window* window, void* id);
CRUI_EXPORT const cr::string16 GetTooltipText(aura::Window* window);
CRUI_EXPORT const void* GetTooltipId(aura::Window* window);

CRUI_EXPORT extern const aura::WindowProperty<void*>* const kTooltipIdKey;
CRUI_EXPORT extern const aura::WindowProperty<cr::string16*>* const
    kTooltipTextKey;

}  // namespace wm
}  // namespace crui

#endif  // UI_WM_PUBLIC_TOOLTIP_CLIENT_H_
