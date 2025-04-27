// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_KEYBOARD_HOOK_BASE_H_
#define UI_EVENTS_KEYBOARD_HOOK_BASE_H_

#include <memory>

#include "crui/events/keyboard_hook.h"

namespace crui {

enum class DomCode;
class KeyEvent;

class KeyboardHookBase : public KeyboardHook {
 public:
  KeyboardHookBase(const KeyboardHookBase&) = delete; 
  KeyboardHookBase& operator=(const KeyboardHookBase&) = delete;

  KeyboardHookBase(cr::Optional<cr::flat_set<DomCode>> dom_codes,
                   KeyEventCallback callback);
  ~KeyboardHookBase() override;

  // KeyboardHook implementation.
  bool IsKeyLocked(DomCode dom_code) const override;

 protected:
  // Indicates whether |dom_code| should be intercepted by the keyboard hook.
  bool ShouldCaptureKeyEvent(DomCode dom_code) const;

  // Forwards the key event using |key_event_callback_|.
  // |event| is owned by the calling method and will live until this method
  // returns.
  void ForwardCapturedKeyEvent(KeyEvent* event);

  const cr::Optional<cr::flat_set<DomCode>>& dom_codes() {
    return dom_codes_;
  }

 private:
  // Used to forward key events.
  KeyEventCallback key_event_callback_;

  // The set of keys which should be intercepted by the keyboard hook.
  cr::Optional<cr::flat_set<DomCode>> dom_codes_;
};

}  // namespace crui

#endif  // UI_EVENTS_KEYBOARD_HOOK_BASE_H_
