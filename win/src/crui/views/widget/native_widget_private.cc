// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/widget/native_widget_private.h"

///#include "crui/base/emoji/emoji_panel_helper.h"
#include "crui/display/display.h"
#include "crui/display/screen.h"

namespace crui {
namespace views {
namespace internal {

// static
gfx::Rect NativeWidgetPrivate::ConstrainBoundsToDisplayWorkArea(
    const gfx::Rect& bounds) {
  gfx::Rect new_bounds(bounds);
  gfx::Rect work_area =
      display::Screen::GetScreen()->GetDisplayMatching(bounds).work_area();
  if (!work_area.IsEmpty())
    new_bounds.AdjustToFit(work_area);
  return new_bounds;
}

void NativeWidgetPrivate::ShowEmojiPanel() {
  ///crui::ShowEmojiPanel();
}

}  // namespace internal
}  // namespace views
}  // namespace crui
