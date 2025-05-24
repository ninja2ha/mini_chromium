// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/client/drag_drop_delegate.h"

#include "crui/base/class_property.h"

DEFINE_EXPORTED_UI_CLASS_PROPERTY_TYPE(CRUI_EXPORT,
                                       crui::aura::client::DragDropDelegate*)

namespace crui {
namespace aura {
namespace client {

DEFINE_UI_CLASS_PROPERTY_KEY(DragDropDelegate*, kDragDropDelegateKey, nullptr)

void SetDragDropDelegate(Window* window, DragDropDelegate* delegate) {
  window->SetProperty(kDragDropDelegateKey, delegate);
}

DragDropDelegate* GetDragDropDelegate(Window* window) {
  return window->GetProperty(kDragDropDelegateKey);
}

}  // namespace client
}  // namespace aura
}  // namespace crui
