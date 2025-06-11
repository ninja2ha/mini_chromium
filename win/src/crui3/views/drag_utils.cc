// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/drag_utils.h"

#include "crui/base/layout.h"
#include "crui/views/widget/widget.h"

namespace crui {
namespace views {

float ScaleFactorForDragFromWidget(const Widget* widget) {
  float device_scale = 1.0f;
  if (widget && widget->GetNativeView()) {
    gfx::NativeView view = widget->GetNativeView();
    device_scale = crui::GetScaleFactorForNativeView(view);
  }
  return device_scale;
}

}  // namespace views
}  // namespace crui
