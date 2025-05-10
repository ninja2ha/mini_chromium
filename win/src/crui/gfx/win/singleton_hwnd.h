// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_WIN_SINGLETON_HWND_H_
#define UI_GFX_WIN_SINGLETON_HWND_H_

#include <windows.h>
#include <vector>

#include "crbase/observer_list.h"
#include "crui/base/ui_export.h"
#include "crui/gfx/win/window_impl.h"

namespace cr {
template<typename T> struct DefaultSingletonTraits;
}  // namespace cr

namespace crui {
namespace gfx {

class SingletonHwndObserver;

// Singleton message-only HWND that allows interested clients to receive WM_*
// notifications.
class CRUI_EXPORT SingletonHwnd : public WindowImpl {
 public:
  static SingletonHwnd* GetInstance();

  // Windows callback for WM_* notifications.
  BOOL ProcessWindowMessage(HWND window,
                            UINT message,
                            WPARAM wparam,
                            LPARAM lparam,
                            LRESULT& result,
                            DWORD msg_map_id) override;

 private:
  friend class SingletonHwndObserver;
  friend struct cr::DefaultSingletonTraits<SingletonHwnd>;

  SingletonHwnd(const SingletonHwnd&) = delete;
  SingletonHwnd& operator=(const SingletonHwnd&) = delete;

  SingletonHwnd();
  ~SingletonHwnd() override;

  // Add/remove SingletonHwndObserver to forward WM_* notifications.
  void AddObserver(SingletonHwndObserver* observer);
  void RemoveObserver(SingletonHwndObserver* observer);

  // List of registered observers.
  ///cr::ObserverList<SingletonHwndObserver, true>::Unchecked observer_list_;
  cr::ObserverList<SingletonHwndObserver, true> observer_list_;
};

}  // namespace gfx
}  // namespace crui

#endif  // UI_GFX_WIN_SINGLETON_HWND_H_
