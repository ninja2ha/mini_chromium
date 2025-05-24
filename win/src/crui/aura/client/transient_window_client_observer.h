// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_AURA_CLIENT_TRANSIENT_WINDOW_CLIENT_OBSERVER_H_
#define UI_AURA_CLIENT_TRANSIENT_WINDOW_CLIENT_OBSERVER_H_

#include "crui/base/ui_export.h"

namespace crui {

namespace aura {

class Window;

namespace client {

class CRUI_EXPORT TransientWindowClientObserver {
 public:
  // Called when a window is added as a transient child. This is called once
  // the child is added as a transient, but before any restacking occurs.
  virtual void OnTransientChildWindowAdded(Window* parent,
                                           Window* transient_child) = 0;

  // Called when a window is removed as a transient child. This is called once
  // the child is removed as a transient, but before any restacking occurs.
  virtual void OnTransientChildWindowRemoved(Window* parent,
                                             Window* transient_child) = 0;

 protected:
  virtual ~TransientWindowClientObserver() {}
};

}  // namespace client
}  // namespace aura

}  // namespace crui

#endif  // UI_AURA_CLIENT_TRANSIENT_WINDOW_CLIENT_OBSERVER_H_
