// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_WIDGET_ROOT_VIEW_TARGETER_H_
#define UI_VIEWS_WIDGET_ROOT_VIEW_TARGETER_H_

#include "crui/views/view_targeter.h"
#include "crui/base/ui_export.h"

namespace crui {
namespace views {

namespace internal {
class RootView;
}  // namespace internal

class View;
class ViewTargeterDelegate;

// A derived class of ViewTargeter that defines targeting logic for cases
// needing to access the members of RootView. For example, when determining the
// target of a gesture event, we need to know if a previous gesture has already
// established the View to which all subsequent gestures should be targeted.
class CRUI_EXPORT RootViewTargeter : public ViewTargeter {
 public:
  RootViewTargeter(const RootViewTargeter&) = delete;
  RootViewTargeter& operator=(const RootViewTargeter&) = delete;

  RootViewTargeter(ViewTargeterDelegate* delegate,
                   internal::RootView* root_view);
  ~RootViewTargeter() override;

 private:
  // ViewTargeter:
  ///View* FindTargetForGestureEvent(View* root,
  ///                                const crui::GestureEvent& gesture) override;
  ///crui::EventTarget* FindNextBestTargetForGestureEvent(
  ///    crui::EventTarget* previous_target,
  ///    const crui::GestureEvent& gesture) override;

  // A pointer to the RootView on which |this| is installed.
  internal::RootView* root_view_;
};

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_WIDGET_ROOT_VIEW_TARGETER_H_
