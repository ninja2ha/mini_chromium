// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/scoped_window_event_targeting_blocker.h"

#include "crui/aura/window.h"

namespace crui {
namespace aura {

ScopedWindowEventTargetingBlocker::ScopedWindowEventTargetingBlocker(
    Window* window)
    : window_(window) {
  if (window_->event_targeting_blocker_count_ == 0) {
    window_->restore_event_targeting_policy_ = window_->event_targeting_policy_;
    window_->SetEventTargetingPolicy(EventTargetingPolicy::kNone);
  }
  // Increase |Window::event_targeting_blocker_count_| after setting the event
  // targeting policy to kNone as Window::SetEventTargetingPolicy() relies on
  // |Window::event_targeting_blocker_count_| to see if the policy is allowed
  // to be changed.
  window_->event_targeting_blocker_count_++;
  window_->AddObserver(this);
}

ScopedWindowEventTargetingBlocker::~ScopedWindowEventTargetingBlocker() {
  if (!window_)
    return;
  window_->RemoveObserver(this);
  window_->event_targeting_blocker_count_--;
  CR_DCHECK(window_->event_targeting_blocker_count_ >= 0);
  if (window_->event_targeting_blocker_count_ == 0)
    window_->SetEventTargetingPolicy(window_->restore_event_targeting_policy_);
}

void ScopedWindowEventTargetingBlocker::OnWindowDestroying(Window* window) {
  CR_DCHECK(window == window_);
  window_->RemoveObserver(this);
  window_ = nullptr;
}

}  // namespace aura
}  // namespace crui
