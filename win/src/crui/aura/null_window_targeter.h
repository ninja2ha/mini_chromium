// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_AURA_NULL_WINDOW_TARGETER_H_
#define UI_AURA_NULL_WINDOW_TARGETER_H_

#include "crui/base/ui_export.h"
#include "crui/aura/window_targeter.h"

namespace crui {
namespace aura {

// NullWindowTargeter can be installed on a root window to prevent it from
// dispatching events such as during shutdown.
class CRUI_EXPORT NullWindowTargeter : public WindowTargeter {
 public:
  NullWindowTargeter(const NullWindowTargeter&) = delete;
  NullWindowTargeter& operator=(const NullWindowTargeter&) = delete;

  NullWindowTargeter();
  ~NullWindowTargeter() override;

  // EventTargeter:
  crui::EventTarget* FindTargetForEvent(crui::EventTarget* root,
                                        crui::Event* event) override;
  crui::EventTarget* FindNextBestTarget(crui::EventTarget* previous_target,
                                        crui::Event* event) override;
};

}  // namespace aura
}  // namespace crui

#endif  // UI_AURA_NULL_WINDOW_TARGETER_H_
