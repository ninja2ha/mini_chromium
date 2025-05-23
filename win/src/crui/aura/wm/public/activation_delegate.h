// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_WM_PUBLIC_ACTIVATION_DELEGATE_H_
#define UI_WM_PUBLIC_ACTIVATION_DELEGATE_H_

#include "crui/base/ui_export.h"

namespace crui {

namespace aura {
class Window;
}  // namespace aura

namespace wm {

// An interface implemented by an object that configures and responds to changes
// to a window's activation state.
class CRUI_EXPORT ActivationDelegate {
 public:
  // Returns true if the window should be activated.
  virtual bool ShouldActivate() const = 0;

 protected:
  virtual ~ActivationDelegate() {}
};

// Sets/Gets the ActivationDelegate on the Window. No ownership changes.
CRUI_EXPORT void SetActivationDelegate(aura::Window* window,
                                       ActivationDelegate* delegate);
CRUI_EXPORT ActivationDelegate* GetActivationDelegate(
    const aura::Window* window);

}  // namespace wm
}  // namespace crui

#endif  // UI_WM_PUBLIC_ACTIVATION_DELEGATE_H_
