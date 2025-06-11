// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_WIDGET_WIDGET_AURA_UTILS_H_
#define UI_VIEWS_WIDGET_WIDGET_AURA_UTILS_H_

#include "crui/aura/client/window_types.h"
#include "crui/views/widget/widget.h"

// Functions shared by native_widget_aura.cc and desktop_native_widget_aura.cc:

namespace crui {
namespace views {

aura::client::WindowType GetAuraWindowTypeForWidgetType(
    Widget::InitParams::Type type);

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_WIDGET_WIDGET_AURA_UTILS_H_
