// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/widget/desktop_aura/desktop_drag_drop_client_win.h"

///#include "base/metrics/histogram_macros.h"
#include "crui/base/dragdrop/drag_drop_types.h"
#include "crui/base/dragdrop/drag_source_win.h"
#include "crui/base/dragdrop/drop_target_event.h"
#include "crui/base/dragdrop/os_exchange_data_provider_win.h"
#include "crui/views/widget/desktop_aura/desktop_drop_target_win.h"
#include "crui/views/widget/desktop_aura/desktop_window_tree_host_win.h"

namespace crui {
namespace views {

DesktopDragDropClientWin::DesktopDragDropClientWin(aura::Window* root_window,
                                                   HWND window)
    : drag_drop_in_progress_(false), drag_operation_(0) {
  drop_target_ = new DesktopDropTargetWin(root_window);
  drop_target_->Init(window);
}

DesktopDragDropClientWin::~DesktopDragDropClientWin() {
  if (drag_drop_in_progress_)
    DragCancel();
}

int DesktopDragDropClientWin::StartDragAndDrop(
    std::unique_ptr<crui::OSExchangeData> data,
    aura::Window* root_window,
    aura::Window* source_window,
    const gfx::Point& screen_location,
    int operation,
    crui::DragDropTypes::DragEventSource source) {
  drag_drop_in_progress_ = true;
  drag_operation_ = operation;

  cr::WeakPtr<DesktopDragDropClientWin> alive(weak_factory_.GetWeakPtr());

  drag_source_ = crui::DragSourceWin::Create();
  Microsoft::WRL::ComPtr<crui::DragSourceWin> drag_source_copy = drag_source_;
  drag_source_copy->set_data(data.get());
  crui::OSExchangeDataProviderWin::GetDataObjectImpl(*data.get())
      ->set_in_drag_loop(true);

  DWORD effect;

  ///UMA_HISTOGRAM_ENUMERATION("Event.DragDrop.Start", source,
  ///                          ui::DragDropTypes::DRAG_EVENT_SOURCE_COUNT);

  HRESULT result = DoDragDrop(
      crui::OSExchangeDataProviderWin::GetIDataObject(*data.get()),
      drag_source_.Get(),
      crui::DragDropTypes::DragOperationToDropEffect(operation), &effect);
  drag_source_copy->set_data(nullptr);

  if (alive)
    drag_drop_in_progress_ = false;

  if (result != DRAGDROP_S_DROP)
    effect = DROPEFFECT_NONE;

  int drag_operation = crui::DragDropTypes::DropEffectToDragOperation(effect);

  ///if (drag_operation == crui::DragDropTypes::DRAG_NONE) {
  ///  UMA_HISTOGRAM_ENUMERATION("Event.DragDrop.Cancel", source,
  ///                            ui::DragDropTypes::DRAG_EVENT_SOURCE_COUNT);
  ///} else {
  ///  UMA_HISTOGRAM_ENUMERATION("Event.DragDrop.Drop", source,
  ///                            ui::DragDropTypes::DRAG_EVENT_SOURCE_COUNT);
  ///}

  return drag_operation;
}

void DesktopDragDropClientWin::DragCancel() {
  drag_source_->CancelDrag();
  drag_operation_ = 0;
}

bool DesktopDragDropClientWin::IsDragDropInProgress() {
  return drag_drop_in_progress_;
}

void DesktopDragDropClientWin::AddObserver(
    aura::client::DragDropClientObserver* observer) {
  CR_NOTIMPLEMENTED();
}

void DesktopDragDropClientWin::RemoveObserver(
    aura::client::DragDropClientObserver* observer) {
  CR_NOTIMPLEMENTED();
}

void DesktopDragDropClientWin::OnNativeWidgetDestroying(HWND window) {
  if (drop_target_.get()) {
    RevokeDragDrop(window);
    drop_target_ = nullptr;
  }
}

}  // namespace views
}  // namespace crui
