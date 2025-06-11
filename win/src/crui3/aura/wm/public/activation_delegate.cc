// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/wm//public/activation_delegate.h"

#include "crui/aura/window.h"
#include "crui/base/class_property.h"

DEFINE_UI_CLASS_PROPERTY_TYPE(crui::wm::ActivationDelegate*)

namespace crui {
namespace wm {

DEFINE_UI_CLASS_PROPERTY_KEY(ActivationDelegate*,
                             kActivationDelegateKey,
                             nullptr)

void SetActivationDelegate(aura::Window* window, ActivationDelegate* delegate) {
  window->SetProperty(kActivationDelegateKey, delegate);
}

ActivationDelegate* GetActivationDelegate(const aura::Window* window) {
  return window->GetProperty(kActivationDelegateKey);
}

}  // namespace wm
}  // namespace crui
