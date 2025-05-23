// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_PLATFORM_WINDOW_PLATFORM_WINDOW_HANDLER_WM_DROP_HANDLER_H_
#define UI_PLATFORM_WINDOW_PLATFORM_WINDOW_HANDLER_WM_DROP_HANDLER_H_

#include <memory>

#include "crui/gfx/native_widget_types.h"
#include "crui/base/ui_export.h"

namespace crui {

namespace gfx {
class PointF;
}  // namespace gfx

class PlatformWindow;
class OSExchangeData;

class CRUI_EXPORT WmDropHandler {
 public:
  // Notifies that dragging is entered to the window. |point| is in the
  // coordinate space of the PlatformWindow.
  virtual void OnDragEnter(const gfx::PointF& point,
                           std::unique_ptr<OSExchangeData> data,
                           int operation) = 0;

  // Notifies that dragging is moved. |widget_out| will be set with the
  // widget located at |point|. |point| is in the coordinate space of the
  // PlatformWindow. It returns the operation selected by client and the
  // returned value should be from ui::DragDropTypes.
  virtual int OnDragMotion(const gfx::PointF& point, int operation) = 0;

  // Notifies that dragged data is dropped. When it doesn't deliver
  // the dragged data on OnDragEnter, it should put it to |data|. The location
  // of the drop is the location of the latest DragEnter/DragMotion. If
  // OSExchangeData is provided on OnDragEnter, the |data| should be same as it.
  virtual void OnDragDrop(std::unique_ptr<crui::OSExchangeData> data) = 0;

  // Notifies that dragging is left.
  virtual void OnDragLeave() = 0;

 protected:
  virtual ~WmDropHandler() {}
};

CRUI_EXPORT void SetWmDropHandler(PlatformWindow* platform_window,
                                  WmDropHandler* drop_handler);
CRUI_EXPORT WmDropHandler* GetWmDropHandler(
    const PlatformWindow& platform_window);

}  // namespace crui

#endif  // UI_PLATFORM_WINDOW_PLATFORM_WINDOW_HANDLER_WM_DROP_HANDLER_H_
