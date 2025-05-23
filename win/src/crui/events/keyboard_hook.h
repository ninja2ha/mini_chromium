// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_KEYBOARD_HOOK_H_
#define UI_EVENTS_KEYBOARD_HOOK_H_

#include <memory>

#include "crbase/functional/callback.h"
#include "crbase/containers/flat_set.h"
#include "crbase/containers/optional.h"
#include "crui/base/ui_export.h"
#include "crui/gfx/native_widget_types.h"

namespace crui {

enum class DomCode;
class KeyEvent;

// Intercepts keyboard events typically handled by the OS or browser.
// Destroying the instance will unregister and clean up the keyboard hook.
class CRUI_EXPORT KeyboardHook {
 public:
  using KeyEventCallback = cr::RepeatingCallback<void(KeyEvent* event)>;

  virtual ~KeyboardHook() = default;

  // Creates a platform specific implementation.
  // |dom_codes| is the set of key codes which will be intercepted, if it is
  // empty, this class will try to intercept all keys allowed by the platform.
  // |callback| is called for each key which is intercepted.
  // Returns a valid instance if the hook was created and successfully
  // registered otherwise nullptr.
  static std::unique_ptr<KeyboardHook> CreateModifierKeyboardHook(
      cr::Optional<cr::flat_set<DomCode>> dom_codes,
      gfx::AcceleratedWidget accelerated_widget,
      KeyEventCallback callback);

  // Creates a platform-specific KeyboardHook implementation that captures the
  // play/pause, stop, and next/previous track media keys.
  // |callback| is called for each key which is intercepted.
  // Returns a valid instance if the hook was created and successfully
  // registered otherwise nullptr.
  static std::unique_ptr<KeyboardHook> CreateMediaKeyboardHook(
      KeyEventCallback callback);

  // True if |dom_code| is reserved for an active KeyboardLock request.
  virtual bool IsKeyLocked(DomCode dom_code) const = 0;
};

}  // namespace crui

#endif  // UI_EVENTS_KEYBOARD_HOOK_H_
