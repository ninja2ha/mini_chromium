// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_WIN_KEYBOARD_HOOK_WIN_BASE_H_
#define UI_EVENTS_WIN_KEYBOARD_HOOK_WIN_BASE_H_

#include <memory>

#include <windows.h>

#include "crbase/logging.h"
#include "crbase/containers/flat_set.h"
#include "crbase/containers/optional.h"
#include "crbase/threading/thread_checker.h"
#include "crui/base/ui_export.h"
#include "crui/events/event.h"
#include "crui/events/keyboard_hook_base.h"
#include "crui/events/keycodes/dom/dom_code.h"

namespace crui {

// Exposes a method to drive the Windows KeyboardHook implementation by feeding
// it key event data.  This method is used by both the low-level keyboard hook
// and by unit tests which simulate the hooked behavior w/o actually installing
// a hook (doing so would cause problems with test parallelization).
class CRUI_EXPORT KeyboardHookWinBase : public KeyboardHookBase {
 public:
  KeyboardHookWinBase(cr::Optional<cr::flat_set<DomCode>> dom_codes,
                      KeyEventCallback callback,
                      bool enable_hook_registration);
  ~KeyboardHookWinBase() override;

  // Create a KeyboardHookWinBase instance which does not register a
  // low-level hook and captures modifier keys.
  static std::unique_ptr<KeyboardHookWinBase>
  CreateModifierKeyboardHookForTesting(
      cr::Optional<cr::flat_set<DomCode>> dom_codes,
      KeyEventCallback callback);

  // Create a KeyboardHookWinBase instance which does not register a
  // low-level hook and captures media keys.
  static std::unique_ptr<KeyboardHookWinBase> CreateMediaKeyboardHookForTesting(
      KeyEventCallback callback);

  // Called when a key event message is delivered via the low-level hook.
  // Exposed here to allow for testing w/o engaging the low-level hook.
  // Returns true if the message was handled.
  virtual bool ProcessKeyEventMessage(WPARAM w_param,
                                      DWORD vk,
                                      DWORD scan_code,
                                      DWORD time_stamp) = 0;

 protected:
  bool Register(HOOKPROC hook_proc);
  bool enable_hook_registration() const { return enable_hook_registration_; }

  static LRESULT CALLBACK ProcessKeyEvent(KeyboardHookWinBase* instance,
                                          int code,
                                          WPARAM w_param,
                                          LPARAM l_param);

 private:
  const bool enable_hook_registration_ = true;
  HHOOK hook_ = nullptr;

#if CR_DCHECK_IS_ON()
  cr::ThreadChecker thread_checker_;
#endif
};

}  // namespace crui

#endif  // UI_EVENTS_WIN_KEYBOARD_HOOK_WIN_BASE_H_
