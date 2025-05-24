// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/wm//public/animation_host.h"

#include "crui/aura/window.h"
#include "crui/base/class_property.h"

DEFINE_UI_CLASS_PROPERTY_TYPE(crui::wm::AnimationHost*)

namespace crui {
namespace wm {

DEFINE_UI_CLASS_PROPERTY_KEY(AnimationHost*, kRootWindowAnimationHostKey, NULL)

void SetAnimationHost(aura::Window* window, AnimationHost* animation_host) {
  CR_DCHECK(window);
  window->SetProperty(kRootWindowAnimationHostKey, animation_host);
}

AnimationHost* GetAnimationHost(aura::Window* window) {
  CR_DCHECK(window);
  return window->GetProperty(kRootWindowAnimationHostKey);
}

}  // namespace wm
}  // namespace crui
