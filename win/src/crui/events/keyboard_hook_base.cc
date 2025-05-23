// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/events/keyboard_hook_base.h"

#include <utility>

#include "crbase/helper/stl_util.h"
#include "crui/events/event.h"
#include "crui/events/keycodes/dom/dom_code.h"

namespace crui {

KeyboardHookBase::KeyboardHookBase(
    cr::Optional<cr::flat_set<DomCode>> dom_codes,
    KeyEventCallback callback)
    : key_event_callback_(std::move(callback)),
      dom_codes_(std::move(dom_codes)) {
  CR_DCHECK(key_event_callback_);
}

KeyboardHookBase::~KeyboardHookBase() = default;

bool KeyboardHookBase::IsKeyLocked(DomCode dom_code) const {
  return ShouldCaptureKeyEvent(dom_code);
}

bool KeyboardHookBase::ShouldCaptureKeyEvent(DomCode dom_code) const {
  if (dom_code == DomCode::NONE)
    return false;

  return !dom_codes_ || cr::Contains(dom_codes_.value(), dom_code);
}

void KeyboardHookBase::ForwardCapturedKeyEvent(KeyEvent* event) {
  key_event_callback_.Run(event);
}

}  // namespace crui
