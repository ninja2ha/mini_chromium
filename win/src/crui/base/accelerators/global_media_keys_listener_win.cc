// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/base/accelerators/global_media_keys_listener_win.h"

#include "crbase/helper/stl_util.h"
#include "crbase/functional/bind.h"
///#include "crbase/metrics/histogram_macros.h"
#include "crui/base/accelerators/accelerator.h"
#include "crui/events/keycodes/keyboard_code_conversion_win.h"
#include "crui/gfx/win/singleton_hwnd_hot_key_observer.h"

namespace crui {

// static
bool GlobalMediaKeysListenerWin::has_instance_ = false;

GlobalMediaKeysListenerWin::GlobalMediaKeysListenerWin(
    MediaKeysListener::Delegate* delegate)
    : delegate_(delegate) {
  CR_DCHECK(delegate_);
  CR_DCHECK(!has_instance_);
  has_instance_ = true;
}

GlobalMediaKeysListenerWin::~GlobalMediaKeysListenerWin() {
  has_instance_ = false;
}

bool GlobalMediaKeysListenerWin::StartWatchingMediaKey(KeyboardCode key_code) {
  CR_DCHECK(IsMediaKeycode(key_code));

  // If the hotkey is already registered, do nothing.
  if (cr::Contains(key_codes_hotkey_observers_, key_code))
    return true;

  // Create an observer that registers a hot key for |key_code|.
  std::unique_ptr<gfx::SingletonHwndHotKeyObserver> observer =
      gfx::SingletonHwndHotKeyObserver::Create(
          cr::BindRepeating(&GlobalMediaKeysListenerWin::OnWndProc,
                            cr::Unretained(this)),
          key_code, /*modifiers=*/0);

  // If observer is null, then the hot key failed to register.
  bool success = !!observer;
  if (success)
    key_codes_hotkey_observers_[key_code] = std::move(observer);

  ///UMA_HISTOGRAM_BOOLEAN("Media.MediaKeysListener.RegisterHotKeyResult",
  ///                      success);

  return success;
}

void GlobalMediaKeysListenerWin::StopWatchingMediaKey(KeyboardCode key_code) {
  CR_DCHECK(IsMediaKeycode(key_code));

  // Deleting the observer automatically unregisters the hot key.
  key_codes_hotkey_observers_.erase(key_code);
}

void GlobalMediaKeysListenerWin::OnWndProc(HWND hwnd,
                                           UINT message,
                                           WPARAM wparam,
                                           LPARAM lparam) {
  // SingletonHwndHotKeyObservers should only send us hot key messages.
  CR_DCHECK(WM_HOTKEY == static_cast<int>(message));

  int win_key_code = HIWORD(lparam);
  KeyboardCode key_code = KeyboardCodeForWindowsKeyCode(win_key_code);

  // We should only receive hot key events for keys that we're observing.
  CR_DCHECK(cr::Contains(key_codes_hotkey_observers_, key_code));

  int modifiers = 0;
  modifiers |= (LOWORD(lparam) & MOD_SHIFT) ? crui::EF_SHIFT_DOWN : 0;
  modifiers |= (LOWORD(lparam) & MOD_ALT) ? crui::EF_ALT_DOWN : 0;
  modifiers |= (LOWORD(lparam) & MOD_CONTROL) ? crui::EF_CONTROL_DOWN : 0;
  Accelerator accelerator(key_code, modifiers);

  delegate_->OnMediaKeysAccelerator(accelerator);
}

}  // namespace crui