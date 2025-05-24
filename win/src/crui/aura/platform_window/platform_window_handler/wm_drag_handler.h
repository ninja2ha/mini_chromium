// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_PLATFORM_WINDOW_PLATFORM_WINDOW_HANDLER_WM_DRAG_HANDLER_H_
#define UI_PLATFORM_WINDOW_PLATFORM_WINDOW_HANDLER_WM_DRAG_HANDLER_H_

#include "crbase/functional/bind.h"
#include "crui/gfx/native_widget_types.h"
#include "crui/base/ui_export.h"

namespace crui {

class PlatformWindow;
class OSExchangeData;

class CRUI_EXPORT WmDragHandler {
 public:
  // Starts dragging with |data| which it wants to deliver to the destination.
  // |operation| is the suggested operation which is bitmask of DRAG_NONE,
  // DRAG_MOVE, DRAG_COPY and DRAG_LINK in DragDropTypes::DragOperation to the
  // destination and the destination sets the final operation when the drop
  // action is performed.
  virtual void StartDrag(const OSExchangeData& data,
                         int operation,
                         gfx::NativeCursor cursor,
                         cr::OnceCallback<void(int)> callback) = 0;

 protected:
  virtual ~WmDragHandler() {}
};

CRUI_EXPORT void SetWmDragHandler(PlatformWindow* platform_window,
                                  WmDragHandler* drag_handler);
CRUI_EXPORT WmDragHandler* GetWmDragHandler(
    const PlatformWindow& platform_window);

}  // namespace crui

#endif  // UI_PLATFORM_WINDOW_PLATFORM_WINDOW_HANDLER_WM_DRAG_HANDLER_H_
