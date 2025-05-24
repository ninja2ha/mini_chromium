// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/scoped_keyboard_hook.h"

#include "crui/aura/window_tree_host.h"
#include "crui/events/keycodes/dom/dom_code.h"

namespace crui {
namespace aura {

ScopedKeyboardHook::ScopedKeyboardHook() = default;

ScopedKeyboardHook::ScopedKeyboardHook(
    cr::WeakPtr<WindowTreeHost> window_tree_host)
    : window_tree_host_(window_tree_host) {
  CR_DCHECK(window_tree_host_);
}

ScopedKeyboardHook::~ScopedKeyboardHook() {
  CR_DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  if (window_tree_host_)
    window_tree_host_->ReleaseSystemKeyEventCapture();
}

bool ScopedKeyboardHook::IsKeyLocked(crui::DomCode dom_code) {
  return window_tree_host_ && window_tree_host_->IsKeyLocked(dom_code);
}

}  // namespace aura
}  // namespace crui
