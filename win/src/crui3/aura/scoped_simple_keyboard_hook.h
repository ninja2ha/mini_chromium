// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_AURA_SCOPED_SIMPLE_KEYBOARD_HOOK_H_
#define UI_AURA_SCOPED_SIMPLE_KEYBOARD_HOOK_H_

#include "crbase/containers/flat_set.h"
#include "crbase/containers/optional.h"
#include "crui/aura/scoped_keyboard_hook.h"

namespace crui {
enum class DomCode;

namespace aura {

// This subclass of ScopedKeyboardHook will not set up a system-level keyboard
// hook or call into any WindowTreeHost methods for lock state or cleanup.
// It allows for disabling system-level keyboard lock functionality while
// continuing to support browser-level keyboard lock.
// TODO(joedow): Remove this class after 'system-keyboard-lock' is removed.
class ScopedSimpleKeyboardHook : public ScopedKeyboardHook {
 public:
  ScopedSimpleKeyboardHook(const ScopedSimpleKeyboardHook&) = delete;
  ScopedSimpleKeyboardHook& operator=(const ScopedSimpleKeyboardHook&) = delete;

  explicit ScopedSimpleKeyboardHook(
      cr::Optional<cr::flat_set<crui::DomCode>> dom_codes);
  ~ScopedSimpleKeyboardHook() override;

  // ScopedKeyboardHook override.
  bool IsKeyLocked(crui::DomCode dom_code) override;

 private:
  cr::Optional<cr::flat_set<crui::DomCode>> dom_codes_;
};

}  // namespace aura
}  // namespace crui

#endif  // UI_AURA_SCOPED_SIMPLE_KEYBOARD_HOOK_H_
