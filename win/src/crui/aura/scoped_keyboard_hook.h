// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_AURA_SCOPED_KEYBOARD_HOOK_H_
#define UI_AURA_SCOPED_KEYBOARD_HOOK_H_

#include "crbase/memory/weak_ptr.h"
#include "crbase/threading/thread_checker.h"
#include "crui/base/ui_export.h"

namespace crui {
enum class DomCode;

namespace aura {

class WindowTreeHost;

// Destroying an instance of this class will clean up the KeyboardHook instance
// owned by WindowTreeHost and prevent future system key events from being
// captured.  If the KeyboardHook or WindowTreeHost instances were already
// destroyed, then destroying this instance is a noop.
class CRUI_EXPORT ScopedKeyboardHook {
 public:
  ScopedKeyboardHook(const ScopedKeyboardHook&) = delete;
  ScopedKeyboardHook& operator=(const ScopedKeyboardHook&) = delete;

  explicit ScopedKeyboardHook(cr::WeakPtr<WindowTreeHost> weak_ptr);
  virtual ~ScopedKeyboardHook();

  // True if |dom_code| is reserved for an active KeyboardLock request.
  virtual bool IsKeyLocked(crui::DomCode dom_code);

 protected:
  ScopedKeyboardHook();

 private:
  CR_THREAD_CHECKER(thread_checker_)

  cr::WeakPtr<WindowTreeHost> window_tree_host_;
};

}  // namespace aura
}  // namespace crui

#endif  // UI_AURA_SCOPED_KEYBOARD_HOOK_H_
