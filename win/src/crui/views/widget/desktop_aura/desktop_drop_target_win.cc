// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/widget/desktop_aura/desktop_drop_target_win.h"

///#include "base/metrics/histogram_macros.h"
#include "crbase/win/win_util.h"
#include "crui/aura/client/drag_drop_client.h"
#include "crui/aura/client/drag_drop_delegate.h"
#include "crui/aura/window.h"
#include "crui/aura/window_tree_host.h"
#include "crui/base/dragdrop/drag_drop_types.h"
#include "crui/base/dragdrop/drop_target_event.h"
#include "crui/base/dragdrop/os_exchange_data_provider_win.h"
#include "crui/events/event_constants.h"

using crui::aura::client::DragDropClient;
using crui::aura::client::DragDropDelegate;
using crui::OSExchangeData;
using crui::OSExchangeDataProviderWin;

namespace {

int ConvertKeyStateToAuraEventFlags(DWORD key_state)
{
  int flags = 0;

  if (key_state & MK_CONTROL)
    flags |= crui::EF_CONTROL_DOWN;
  if (key_state & MK_ALT)
    flags |= crui::EF_ALT_DOWN;
  if (key_state & MK_SHIFT)
    flags |= crui::EF_SHIFT_DOWN;
  if (key_state & MK_LBUTTON)
    flags |= crui::EF_LEFT_MOUSE_BUTTON;
  if (key_state & MK_MBUTTON)
    flags |= crui::EF_MIDDLE_MOUSE_BUTTON;
  if (key_state & MK_RBUTTON)
    flags |= crui::EF_RIGHT_MOUSE_BUTTON;

  return flags;
}

}  // namespace

namespace crui {
namespace views {

DesktopDropTargetWin::DesktopDropTargetWin(aura::Window* root_window)
    : root_window_(root_window), target_window_(nullptr) {}

DesktopDropTargetWin::~DesktopDropTargetWin() {
  if (target_window_)
    target_window_->RemoveObserver(this);
}

DWORD DesktopDropTargetWin::OnDragEnter(IDataObject* data_object,
                                        DWORD key_state,
                                        POINT position,
                                        DWORD effect) {
  std::unique_ptr<OSExchangeData> data;
  std::unique_ptr<crui::DropTargetEvent> event;
  DragDropDelegate* delegate;
  // Translate will call OnDragEntered.
  Translate(data_object, key_state, position, effect, &data, &event, &delegate);
  return crui::DragDropTypes::DragOperationToDropEffect(
      crui::DragDropTypes::DRAG_NONE);
}

DWORD DesktopDropTargetWin::OnDragOver(IDataObject* data_object,
                                       DWORD key_state,
                                       POINT position,
                                       DWORD effect) {
  int drag_operation = crui::DragDropTypes::DRAG_NONE;
  std::unique_ptr<OSExchangeData> data;
  std::unique_ptr<crui::DropTargetEvent> event;
  DragDropDelegate* delegate;
  Translate(data_object, key_state, position, effect, &data, &event, &delegate);
  if (delegate)
    drag_operation = delegate->OnDragUpdated(*event);

  ///UMA_HISTOGRAM_BOOLEAN("Event.DragDrop.AcceptDragUpdate",
  ///                      drag_operation != ui::DragDropTypes::DRAG_NONE);
  return crui::DragDropTypes::DragOperationToDropEffect(drag_operation);
}

void DesktopDropTargetWin::OnDragLeave(IDataObject* data_object) {
  NotifyDragLeave();
}

DWORD DesktopDropTargetWin::OnDrop(IDataObject* data_object,
                                   DWORD key_state,
                                   POINT position,
                                   DWORD effect) {
  int drag_operation = crui::DragDropTypes::DRAG_NONE;
  std::unique_ptr<OSExchangeData> data;
  std::unique_ptr<crui::DropTargetEvent> event;
  DragDropDelegate* delegate;
  Translate(data_object, key_state, position, effect, &data, &event, &delegate);
  if (delegate) {
    drag_operation = delegate->OnPerformDrop(*event, std::move(data));
    DragDropClient* client = aura::client::GetDragDropClient(root_window_);
    if (client && !client->IsDragDropInProgress() &&
        drag_operation != crui::DragDropTypes::DRAG_NONE) {
      ///UMA_HISTOGRAM_COUNTS_1M("Event.DragDrop.ExternalOriginDrop", 1);
    }
  }
  if (target_window_) {
    target_window_->RemoveObserver(this);
    target_window_ = nullptr;
  }
  return crui::DragDropTypes::DragOperationToDropEffect(drag_operation);
}

void DesktopDropTargetWin::OnWindowDestroyed(aura::Window* window) {
  CR_DCHECK(window == target_window_);
  target_window_ = nullptr;
}

void DesktopDropTargetWin::Translate(
    IDataObject* data_object,
    DWORD key_state,
    POINT position,
    DWORD effect,
    std::unique_ptr<OSExchangeData>* data,
    std::unique_ptr<crui::DropTargetEvent>* event,
    DragDropDelegate** delegate) {
  gfx::Point location(position.x, position.y);
  gfx::Point root_location = location;
  root_window_->GetHost()->ConvertScreenInPixelsToDIP(&root_location);
  aura::Window* target_window =
      root_window_->GetEventHandlerForPoint(root_location);
  bool target_window_changed = false;
  if (target_window != target_window_) {
    if (target_window_)
      NotifyDragLeave();
    target_window_ = target_window;
    if (target_window_)
      target_window_->AddObserver(this);
    target_window_changed = true;
  }
  *delegate = nullptr;
  if (!target_window_)
    return;
  *delegate = aura::client::GetDragDropDelegate(target_window_);
  if (!*delegate)
    return;

  *data = std::make_unique<OSExchangeData>(
      std::make_unique<OSExchangeDataProviderWin>(data_object));
  location = root_location;
  aura::Window::ConvertPointToTarget(root_window_, target_window_, &location);
  *event = std::make_unique<crui::DropTargetEvent>(
      *(data->get()), gfx::PointF(location), gfx::PointF(root_location),
      crui::DragDropTypes::DropEffectToDragOperation(effect));
  (*event)->set_flags(ConvertKeyStateToAuraEventFlags(key_state));
  if (target_window_changed)
    (*delegate)->OnDragEntered(*event->get());
}

void DesktopDropTargetWin::NotifyDragLeave() {
  if (!target_window_)
    return;
  DragDropDelegate* delegate =
      aura::client::GetDragDropDelegate(target_window_);
  if (delegate)
    delegate->OnDragExited();
  target_window_->RemoveObserver(this);
  target_window_ = nullptr;
}

}  // namespace views
}  // namespace crui
