// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_AURA_SCOPED_WINDOW_EVENT_TARGETING_BLOCKER_H_
#define UI_AURA_SCOPED_WINDOW_EVENT_TARGETING_BLOCKER_H_

#include "crui/base/ui_export.h"
#include "crui/aura/window_observer.h"

namespace crui {
namespace aura {

class Window;

// Temporarily blocks the event targeting by setting kNone targeting policy to
// |window_|. The original event targeting policy will be restored if all
// targeting blockers are removed from |window_|.
class CRUI_EXPORT ScopedWindowEventTargetingBlocker : public WindowObserver {
 public:
  ScopedWindowEventTargetingBlocker(
      const ScopedWindowEventTargetingBlocker&) = delete;
  ScopedWindowEventTargetingBlocker& operator=(
      const ScopedWindowEventTargetingBlocker&) = delete;

  explicit ScopedWindowEventTargetingBlocker(Window* window);
  ~ScopedWindowEventTargetingBlocker() override;

  // WindowObserver:
  void OnWindowDestroying(Window* window) override;

 private:
  Window* window_;
};

}  // namespace aura
}  // namespace crui

#endif  // UI_AURA_SCOPED_WINDOW_EVENT_TARGETING_BLOCKER_H_
