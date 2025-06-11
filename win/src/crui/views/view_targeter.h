// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_VIEW_TARGETER_H_
#define UI_VIEWS_VIEW_TARGETER_H_

#include "crui/events/event_targeter.h"
#include "crui/base/ui_export.h"

namespace crui {

///class GestureEvent;
class KeyEvent;
class ScrollEvent;

namespace gfx {
class Rect;
}  // namespace gfx

namespace views {

class View;
class ViewTargeterDelegate;

// A ViewTargeter is installed on a View that wishes to use the custom
// hit-testing or event-targeting behaviour defined by |delegate|.
class CRUI_EXPORT ViewTargeter : public crui::EventTargeter {
 public:
  ViewTargeter(const ViewTargeter&) = delete;
  ViewTargeter& operator=(const ViewTargeter&) = delete;

  explicit ViewTargeter(ViewTargeterDelegate* delegate);
  ~ViewTargeter() override;

  // A call-through to DoesIntersectRect() on |delegate_|.
  bool DoesIntersectRect(const View* target, const gfx::Rect& rect) const;

  // A call-through to TargetForRect() on |delegate_|.
  View* TargetForRect(View* root, const gfx::Rect& rect) const;

 protected:
  // ui::EventTargeter:
  crui::EventTarget* FindTargetForEvent(crui::EventTarget* root,
                                        crui::Event* event) override;
  crui::EventTarget* FindNextBestTarget(crui::EventTarget* previous_target,
                                        crui::Event* event) override;

 private:
  View* FindTargetForKeyEvent(View* root, const crui::KeyEvent& key);
  View* FindTargetForScrollEvent(View* root, const crui::ScrollEvent& scroll);

  ///virtual View* FindTargetForGestureEvent(View* root,
  ///                                        const crui::GestureEvent& gesture);
  ///virtual crui::EventTarget* FindNextBestTargetForGestureEvent(
  ///    crui::EventTarget* previous_target,
  ///    const crui::GestureEvent& gesture);
  
  // ViewTargeter does not own the |delegate_|, but |delegate_| must
  // outlive the targeter.
  ViewTargeterDelegate* delegate_;
};

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_VIEW_TARGETER_H_
