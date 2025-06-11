// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_ACCELERATORS_GLOBAL_MEDIA_KEYS_LISTENER_WIN_H_
#define UI_BASE_ACCELERATORS_GLOBAL_MEDIA_KEYS_LISTENER_WIN_H_

#include <windows.h>

#include "crbase/containers/flat_map.h"
#include "crui/base/accelerators/media_keys_listener.h"
#include "crui/base/ui_export.h"
#include "crui/events/keycodes/keyboard_codes.h"

namespace crui {

namespace gfx {
class SingletonHwndHotKeyObserver;
}  // namespace gfx

// Implementation of MediaKeysListener that uses RegisterHotKey to globally
// listen for media key presses. It only allows for a single instance to be
// created in order to prevent conflicts form multiple listeners.
class CRUI_EXPORT GlobalMediaKeysListenerWin : public MediaKeysListener {
 public:
  GlobalMediaKeysListenerWin(
      const GlobalMediaKeysListenerWin&) = delete;
  GlobalMediaKeysListenerWin& operator=(
      const GlobalMediaKeysListenerWin&) = delete;

  explicit GlobalMediaKeysListenerWin(MediaKeysListener::Delegate* delegate);
  ~GlobalMediaKeysListenerWin() override;

  static bool has_instance() { return has_instance_; }

  // MediaKeysListener implementation.
  bool StartWatchingMediaKey(KeyboardCode key_code) override;
  void StopWatchingMediaKey(KeyboardCode key_code) override;
  void SetIsMediaPlaying(bool is_playing) override {}

 private:
  // Called by SingletonHwndObserver.
  void OnWndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

  static bool has_instance_;

  MediaKeysListener::Delegate* delegate_;
  cr::flat_map<KeyboardCode,
               std::unique_ptr<gfx::SingletonHwndHotKeyObserver>>
      key_codes_hotkey_observers_;
};

}  // namespace crui

#endif  // UI_BASE_ACCELERATORS_GLOBAL_MEDIA_KEYS_LISTENER_WIN_H_
