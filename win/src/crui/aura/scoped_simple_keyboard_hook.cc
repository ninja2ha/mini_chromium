// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/scoped_simple_keyboard_hook.h"

#include <utility>

#include "crbase/helper/stl_util.h"
#include "crui/events/keycodes/dom/dom_code.h"

namespace crui {
namespace aura {

ScopedSimpleKeyboardHook::ScopedSimpleKeyboardHook(
    cr::Optional<cr::flat_set<crui::DomCode>> dom_codes)
    : dom_codes_(std::move(dom_codes)) {}

ScopedSimpleKeyboardHook::~ScopedSimpleKeyboardHook() = default;

bool ScopedSimpleKeyboardHook::IsKeyLocked(crui::DomCode dom_code) {
  if (dom_code == crui::DomCode::NONE)
    return false;

  return !dom_codes_ || cr::Contains(dom_codes_.value(), dom_code);
}

}  // namespace aura
}  // namespace crui
