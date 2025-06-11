// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_DRAG_UTILS_H_
#define UI_VIEWS_DRAG_UTILS_H_

#include <memory>

#include "crui/base/dragdrop/drag_drop_types.h"
#include "crui/base/dragdrop/os_exchange_data.h"
#include "crui/gfx/native_widget_types.h"
#include "crui/base/ui_export.h"

namespace crui {

namespace gfx {
class Point;
}  // namespace gfx

namespace views {
class Widget;

// Starts a drag operation. This blocks until the drag operation completes.
CRUI_EXPORT void RunShellDrag(gfx::NativeView view,
                              std::unique_ptr<crui::OSExchangeData> data,
                              const gfx::Point& location,
                              int operation,
                              crui::DragDropTypes::DragEventSource source);

// Returns the device scale for the display associated with this |widget|'s
// native view.
CRUI_EXPORT float ScaleFactorForDragFromWidget(const Widget* widget);

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_DRAG_UTILS_H_
