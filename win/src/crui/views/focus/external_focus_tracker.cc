// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/focus/external_focus_tracker.h"

#include "crbase/logging.h"
#include "crui/views/view.h"
#include "crui/views/view_tracker.h"

namespace crui {
namespace views {

ExternalFocusTracker::ExternalFocusTracker(View* parent_view,
                                           FocusManager* focus_manager)
    : focus_manager_(focus_manager),
      parent_view_(parent_view),
      last_focused_view_tracker_(std::make_unique<ViewTracker>()) {
  CR_DCHECK(parent_view);
  // Store the view which is focused when we're created.
  if (focus_manager_)
    StartTracking();
}

ExternalFocusTracker::~ExternalFocusTracker() {
  if (focus_manager_)
    focus_manager_->RemoveFocusChangeListener(this);
}

void ExternalFocusTracker::OnWillChangeFocus(View* focused_before,
                                             View* focused_now) {
  if (focused_now && !parent_view_->Contains(focused_now) &&
      parent_view_ != focused_now) {
    // Store the newly focused view.
    StoreLastFocusedView(focused_now);
  }
}

void ExternalFocusTracker::OnDidChangeFocus(View* focused_before,
                                            View* focused_now) {
}

void ExternalFocusTracker::FocusLastFocusedExternalView() {
  View* last_focused_view = last_focused_view_tracker_->view();
  if (last_focused_view)
    last_focused_view->RequestFocus();
}

void ExternalFocusTracker::SetFocusManager(FocusManager* focus_manager) {
  if (focus_manager_)
    focus_manager_->RemoveFocusChangeListener(this);
  focus_manager_ = focus_manager;
  if (focus_manager_)
    StartTracking();
}

void ExternalFocusTracker::StoreLastFocusedView(View* view) {
  last_focused_view_tracker_->SetView(view);
}

void ExternalFocusTracker::StartTracking() {
  StoreLastFocusedView(focus_manager_->GetFocusedView());
  focus_manager_->AddFocusChangeListener(this);
}

}  // namespace views
}  // namespace crui
