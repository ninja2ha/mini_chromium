// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_AURA_ENV_INPUT_STATE_CONTROLLER_H_
#define UI_AURA_ENV_INPUT_STATE_CONTROLLER_H_

#include <stdint.h>

#include "crui/base/ui_export.h"

namespace crui {

class MouseEvent;
class TouchEvent;

namespace gfx {
class Point;
}  // namespace gfx

namespace aura {

class Env;
class Window;

class CRUI_EXPORT EnvInputStateController {
 public:
  EnvInputStateController(const EnvInputStateController&) = delete;
  EnvInputStateController& operator=(const EnvInputStateController&) = delete;

  explicit EnvInputStateController(Env* env);
  ~EnvInputStateController();

  void UpdateStateForMouseEvent(const Window* window,
                                const crui::MouseEvent& event);
  void UpdateStateForTouchEvent(const crui::TouchEvent& event);
  void SetLastMouseLocation(const Window* root_window,
                            const gfx::Point& location_in_root) const;

 private:
  Env* env_;
  // Touch ids that are currently down.
  uint32_t touch_ids_down_ = 0;
};

}  // namespace aura
}  // namespace crui

#endif  // UI_AURA_ENV_INPUT_STATE_CONTROLLER_H_
