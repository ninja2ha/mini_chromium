// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_AURA_CLIENT_FOCUS_CHANGE_OBSERVER_H_
#define UI_AURA_CLIENT_FOCUS_CHANGE_OBSERVER_H_

#include "crui/base/ui_export.h"

namespace crui {

namespace aura {
class Window;
namespace client {

// TODO(beng): this interface will be OBSOLETE by FocusChangeEvent.
class CRUI_EXPORT FocusChangeObserver {
 public:
  // Called when focus moves from |lost_focus| to |gained_focus|.
  virtual void OnWindowFocused(Window* gained_focus, Window* lost_focus) = 0;

 protected:
  virtual ~FocusChangeObserver() {}
};

CRUI_EXPORT FocusChangeObserver* GetFocusChangeObserver(Window* window);
CRUI_EXPORT void SetFocusChangeObserver(
    Window* window,
    FocusChangeObserver* focus_change_observer);


}  // namespace client
}  // namespace aura

}  // namespace crui

#endif  // UI_AURA_CLIENT_FOCUS_CHANGE_OBSERVER_H_
