// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/wm//public/activation_change_observer.h"

#include "crui/aura/window.h"
#include "crui/base/class_property.h"

DEFINE_UI_CLASS_PROPERTY_TYPE(crui::wm::ActivationChangeObserver*)

namespace crui {
namespace wm {

DEFINE_UI_CLASS_PROPERTY_KEY(ActivationChangeObserver*,
                             kActivationChangeObserverKey,
                             nullptr)

void SetActivationChangeObserver(aura::Window* window,
                                 ActivationChangeObserver* observer) {
  window->SetProperty(kActivationChangeObserverKey, observer);
}

ActivationChangeObserver* GetActivationChangeObserver(aura::Window* window) {
  return window ? window->GetProperty(kActivationChangeObserverKey) : NULL;
}

}  // namespace wm
}  // namespace crui
