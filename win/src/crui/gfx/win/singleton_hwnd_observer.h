// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_WIN_SINGLETON_HWND_OBSERVER_H_
#define UI_GFX_WIN_SINGLETON_HWND_OBSERVER_H_

#include <windows.h>

#include "crbase/functional/callback.h"
#include "crui/base/ui_export.h"

namespace crui {
namespace gfx {

class SingletonHwnd;

// Singleton lifetime management is tricky. This observer handles the correct
// cleanup if either the SingletonHwnd or forwarded object is destroyed first.
// Note that if you want to register a hot key on the SingletonHwnd, you need to
// use a SingletonHwndHotKeyObserver instead for each hot key.
class CRUI_EXPORT SingletonHwndObserver {
 public:
  using WndProc = cr::RepeatingCallback<void(HWND, UINT, WPARAM, LPARAM)>;

  SingletonHwndObserver(const SingletonHwndObserver&) = delete;
  SingletonHwndObserver& operator=(const SingletonHwndObserver&) = delete;

  explicit SingletonHwndObserver(const WndProc& wnd_proc);
  ~SingletonHwndObserver();

 private:
  friend class SingletonHwnd;

  void ClearWndProc();
  void OnWndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

  WndProc wnd_proc_;
};

}  // namespace gfx
}  // namespace crui

#endif  // UI_GFX_WIN_SINGLETON_HWND_OBSERVER_H_
