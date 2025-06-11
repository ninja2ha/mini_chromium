// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/widget/focus_manager_event_handler.h"

#include "crui/aura/window.h"
#include "crui/views/focus/focus_manager.h"
#include "crui/views/widget/widget.h"

namespace crui {
namespace views {

FocusManagerEventHandler::FocusManagerEventHandler(Widget* widget,
                                                   aura::Window* window)
    : widget_(widget), window_(window) {
  CR_DCHECK(window_);
  window_->AddPreTargetHandler(this);
}

FocusManagerEventHandler::~FocusManagerEventHandler() {
  window_->RemovePreTargetHandler(this);
}

void FocusManagerEventHandler::OnKeyEvent(crui::KeyEvent* event) {
  if (widget_ && widget_->GetFocusManager()->GetFocusedView() &&
      !widget_->GetFocusManager()->OnKeyEvent(*event)) {
    event->StopPropagation();
  }
}

}  // namespace views
}  // namespace crui
