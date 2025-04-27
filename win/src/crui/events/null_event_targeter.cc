// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/events/null_event_targeter.h"

#include "crbase/logging.h"

namespace crui {

NullEventTargeter::NullEventTargeter() {
}

NullEventTargeter::~NullEventTargeter() {
}

EventTarget* NullEventTargeter::FindTargetForEvent(EventTarget* root,
                                                   Event* event) {
  return nullptr;
}

EventTarget* NullEventTargeter::FindNextBestTarget(EventTarget* previous_target,
                                                   Event* event) {
  CR_NOTREACHED();
  return nullptr;
}

}  // namespace crui
