// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_WIN_SINGLETON_HWND_HOT_KEY_OBSERVER_H_
#define UI_GFX_WIN_SINGLETON_HWND_HOT_KEY_OBSERVER_H_

#include <windows.h>

#include <memory>

#include "crui/base/ui_export.h"
#include "crui/gfx/win/singleton_hwnd_observer.h"

namespace crui {
namespace gfx {

// We need to avoid duplicate hot key IDs for SingletonHwndObservers that call
// RegisterHotKey for SingletonHwnd. This class properly handles getting a
// unique hot key ID, registers the hotkey on construction, and unregisters the
// hot key on destruction.
//
// This class should always be used instead of directly registering hot keys on
// the SingletonHwnd with a SingletonHwndObserver in order to prevent duplicate
// hot key IDs.
class CRUI_EXPORT SingletonHwndHotKeyObserver {
 public:
  SingletonHwndHotKeyObserver(
      const SingletonHwndHotKeyObserver&) = delete;
  SingletonHwndHotKeyObserver& operator=(
      const SingletonHwndHotKeyObserver&) = delete;
  // Registers a hot key with the given |key_code| and |modifiers| and returns
  // a SingletonHwndHotKeyObserver if successful. Returns null if the hot key
  // fails to register, which can happen if another application has already
  // registered the hot key.
  static std::unique_ptr<SingletonHwndHotKeyObserver> Create(
      const SingletonHwndObserver::WndProc& wnd_proc,
      UINT key_code,
      int modifiers);
  ~SingletonHwndHotKeyObserver();

 private:
  SingletonHwndHotKeyObserver(const SingletonHwndObserver::WndProc& wnd_proc,
                              int hot_key_id);

  // Called by SingletonHwndObserver.
  void OnWndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

  SingletonHwndObserver observer_;
  SingletonHwndObserver::WndProc wnd_proc_;
  const int hot_key_id_;
};

}  // namespace gfx
}  // namespace crui

#endif  // UI_GFX_WIN_SINGLETON_HWND_HOT_KEY_OBSERVER_H_
