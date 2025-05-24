// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_DRAG_DROP_CLIENT_WIN_H_
#define UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_DRAG_DROP_CLIENT_WIN_H_

#include <wrl/client.h>

#include "crbase/compiler_specific.h"
#include "crbase/memory/ref_counted.h"
#include "crbase/memory/weak_ptr.h"
#include "crui/aura/client/drag_drop_client.h"
#include "crui/base/ui_export.h"

namespace crui {

class DragSourceWin;

namespace aura {
namespace client {
class DragDropClientObserver;
}  // namespace client
}  // namespace aura

namespace views {
class DesktopDropTargetWin;

class CRUI_EXPORT DesktopDragDropClientWin
    : public aura::client::DragDropClient {
 public:
  DesktopDragDropClientWin(const DesktopDragDropClientWin&) = delete;
  DesktopDragDropClientWin& operator=(
      const DesktopDragDropClientWin&) = delete;

  DesktopDragDropClientWin(aura::Window* root_window, HWND window);
  ~DesktopDragDropClientWin() override;

  // Overridden from aura::client::DragDropClient:
  int StartDragAndDrop(std::unique_ptr<crui::OSExchangeData> data,
                       aura::Window* root_window,
                       aura::Window* source_window,
                       const gfx::Point& screen_location,
                       int operation,
                       crui::DragDropTypes::DragEventSource source) override;
  void DragCancel() override;
  bool IsDragDropInProgress() override;
  void AddObserver(aura::client::DragDropClientObserver* observer) override;
  void RemoveObserver(aura::client::DragDropClientObserver* observer) override;

  void OnNativeWidgetDestroying(HWND window);

 private:
  bool drag_drop_in_progress_;

  int drag_operation_;

  Microsoft::WRL::ComPtr<crui::DragSourceWin> drag_source_;

  cr::RefPtr<DesktopDropTargetWin> drop_target_;

  cr::WeakPtrFactory<DesktopDragDropClientWin> weak_factory_{this};
};

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_WIDGET_DESKTOP_AURA_DESKTOP_DRAG_DROP_CLIENT_WIN_H_
