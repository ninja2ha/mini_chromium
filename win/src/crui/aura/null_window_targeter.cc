// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/null_window_targeter.h"

#include "crbase/logging.h"

namespace crui {
namespace aura {

NullWindowTargeter::NullWindowTargeter() = default;
NullWindowTargeter::~NullWindowTargeter() = default;

crui::EventTarget* NullWindowTargeter::FindTargetForEvent(
    crui::EventTarget* root,
    crui::Event* event) {
  return nullptr;
}

crui::EventTarget* NullWindowTargeter::FindNextBestTarget(
    crui::EventTarget* previous_target,
    crui::Event* event) {
  CR_NOTREACHED();
  return nullptr;
}

}  // namespace aura
}  // namespace crui
