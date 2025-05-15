// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_ANIMATION_INK_DROP_EVENT_HANDLER_H_
#define UI_VIEWS_ANIMATION_INK_DROP_EVENT_HANDLER_H_

#include <memory>

#include "crbase/scoped_observer.h"
#include "crui/events/event_handler.h"
#include "crui/views/view.h"
#include "crui/views/view_observer.h"
#include "crui/base/ui_export.h"

namespace crui {

class LocatedEvent;
class ScopedTargetHandler;

namespace views {
class InkDrop;
enum class InkDropState;
struct ViewHierarchyChangedDetails;

// This class handles ink-drop changes due to events on its host.
class CRUI_EXPORT InkDropEventHandler : public crui::EventHandler,
                                        public ViewObserver {
 public:
  // Delegate class that allows InkDropEventHandler to be used with InkDrops
  // that are hosted in multiple ways.
  class Delegate {
   public:
    // Gets the InkDrop (or stub) that should react to incoming events.
    virtual InkDrop* GetInkDrop() = 0;

    virtual bool HasInkDrop() const = 0;

    // Returns true if gesture events should affect the InkDrop.
    virtual bool SupportsGestureEvents() const = 0;
  };

  InkDropEventHandler(const InkDropEventHandler&) = delete;
  InkDropEventHandler& operator=(const InkDropEventHandler&) = delete;

  InkDropEventHandler(View* host_view, Delegate* delegate);
  ~InkDropEventHandler() override;

  void AnimateInkDrop(InkDropState state, const crui::LocatedEvent* event);
  crui::LocatedEvent* GetLastRippleTriggeringEvent() const;

 private:
  // ui::EventHandler:
  ///void OnGestureEvent(crui::GestureEvent* event) override;
  void OnMouseEvent(crui::MouseEvent* event) override;

  // ViewObserver:
  void OnViewVisibilityChanged(View* observed_view,
                               View* starting_view) override;
  void OnViewHierarchyChanged(
      View* observed_view,
      const ViewHierarchyChangedDetails& details) override;
  void OnViewBoundsChanged(View* observed_view) override;
  void OnViewFocused(View* observed_view) override;
  void OnViewBlurred(View* observed_view) override;

  // Allows |this| to handle all GestureEvents on |host_view_|.
  std::unique_ptr<crui::ScopedTargetHandler> target_handler_;

  // The host view.
  View* const host_view_;

  // Delegate used to get the InkDrop, etc.
  Delegate* const delegate_;

  // The last user Event to trigger an InkDrop-ripple animation.
  std::unique_ptr<crui::LocatedEvent> last_ripple_triggering_event_;

  cr::ScopedObserver<View, ViewObserver> observer_{this};
};

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_ANIMATION_INK_DROP_EVENT_HANDLER_H_
