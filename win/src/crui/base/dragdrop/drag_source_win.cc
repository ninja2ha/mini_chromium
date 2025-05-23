// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/base/dragdrop/drag_source_win.h"

///#include "crui/base/dragdrop/os_exchange_data_provider_win.h"

namespace crui {

Microsoft::WRL::ComPtr<crui::DragSourceWin> DragSourceWin::Create() {
  ///return Microsoft::WRL::Make<crui::DragSourceWin>();
  return nullptr;
}

DragSourceWin::DragSourceWin() : cancel_drag_(false), data_(nullptr) {
}

HRESULT DragSourceWin::QueryContinueDrag(BOOL escape_pressed, DWORD key_state) {
  if (cancel_drag_)
    return DRAGDROP_S_CANCEL;

  if (escape_pressed) {
    OnDragSourceCancel();
    return DRAGDROP_S_CANCEL;
  }

  if (!(key_state & MK_LBUTTON)) {
    OnDragSourceDrop();
    return DRAGDROP_S_DROP;
  }

  OnDragSourceMove();
  return S_OK;
}

HRESULT DragSourceWin::GiveFeedback(DWORD effect) {
  return DRAGDROP_S_USEDEFAULTCURSORS;
}

void DragSourceWin::OnDragSourceDrop() {
  CR_DCHECK(data_);
  ///crui::OSExchangeDataProviderWin::GetDataObjectImpl(*data_)
  ///    ->set_in_drag_loop(false);
}

}  // namespace crui
