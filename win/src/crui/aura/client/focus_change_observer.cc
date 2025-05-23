// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/client/focus_change_observer.h"

#include "crui/aura/window.h"
#include "crui/base/class_property.h"

DEFINE_UI_CLASS_PROPERTY_TYPE(crui::aura::client::FocusChangeObserver*)

namespace crui {

namespace aura {
namespace client {

DEFINE_UI_CLASS_PROPERTY_KEY(FocusChangeObserver*,
                             kFocusChangeObserverKey,
                             nullptr)

FocusChangeObserver* GetFocusChangeObserver(Window* window) {
  return window ? window->GetProperty(kFocusChangeObserverKey) : NULL;
}

void SetFocusChangeObserver(Window* window,
                            FocusChangeObserver* focus_change_observer) {
  window->SetProperty(kFocusChangeObserverKey, focus_change_observer);
}

}  // namespace client
}  // namespace aura
}  // namespace crui
