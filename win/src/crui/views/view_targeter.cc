// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/view_targeter.h"

#include "crui/events/event_target.h"
#include "crui/views/focus/focus_manager.h"
#include "crui/views/view.h"
#include "crui/views/view_targeter_delegate.h"

namespace crui {
namespace views {

ViewTargeter::ViewTargeter(ViewTargeterDelegate* delegate)
    : delegate_(delegate) {
  CR_DCHECK(delegate_);
}

ViewTargeter::~ViewTargeter() = default;

bool ViewTargeter::DoesIntersectRect(const View* target,
                                     const gfx::Rect& rect) const {
  return delegate_->DoesIntersectRect(target, rect);
}

View* ViewTargeter::TargetForRect(View* root, const gfx::Rect& rect) const {
  return delegate_->TargetForRect(root, rect);
}

crui::EventTarget* ViewTargeter::FindTargetForEvent(crui::EventTarget* root,
                                                    crui::Event* event) {
  View* view = static_cast<View*>(root);

  if (event->IsKeyEvent())
    return FindTargetForKeyEvent(view, *event->AsKeyEvent());

  if (event->IsScrollEvent())
    return FindTargetForScrollEvent(view, *event->AsScrollEvent());

  if (event->IsGestureEvent()) {
    crui::GestureEvent* gesture = event->AsGestureEvent();
    View* gesture_target = FindTargetForGestureEvent(view, *gesture);
    root->ConvertEventToTarget(gesture_target, gesture);
    return gesture_target;
  }

  CR_NOTREACHED() << "ViewTargeter does not yet support this event type.";
  return nullptr;
}

crui::EventTarget* ViewTargeter::FindNextBestTarget(
    crui::EventTarget* previous_target,
    crui::Event* event) {
  if (!previous_target)
    return nullptr;

  if (event->IsGestureEvent()) {
    crui::GestureEvent* gesture = event->AsGestureEvent();
    crui::EventTarget* next_target =
        FindNextBestTargetForGestureEvent(previous_target, *gesture);
    previous_target->ConvertEventToTarget(next_target, gesture);
    return next_target;
  }

  return previous_target->GetParentTarget();
}

View* ViewTargeter::FindTargetForKeyEvent(View* root, const 
                                          crui::KeyEvent& key) {
  if (root->GetFocusManager())
    return root->GetFocusManager()->GetFocusedView();
  return nullptr;
}

View* ViewTargeter::FindTargetForScrollEvent(View* root,
                                             const crui::ScrollEvent& scroll) {
  gfx::Rect rect(scroll.location(), gfx::Size(1, 1));
  return root->GetEffectiveViewTargeter()->TargetForRect(root, rect);
}

View* ViewTargeter::FindTargetForGestureEvent(
    View* root,
    const crui::GestureEvent& gesture) {
  // TODO(tdanderson): The only code path that performs targeting for gestures
  //                   uses the ViewTargeter installed on the RootView (i.e.,
  //                   a RootViewTargeter). Provide a default implementation
  //                   here if we need to be able to perform gesture targeting
  //                   starting at an arbitrary node in a Views tree.
  CR_NOTREACHED();
  return nullptr;
}

crui::EventTarget* ViewTargeter::FindNextBestTargetForGestureEvent(
    crui::EventTarget* previous_target,
    const crui::GestureEvent& gesture) {
  CR_NOTREACHED();
  return nullptr;
}

}  // namespace views
}  // namespace crui
