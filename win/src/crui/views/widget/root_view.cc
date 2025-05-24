// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/widget/root_view.h"

#include <algorithm>

#include "crbase/logging.h"
///#include "crui/accessibility/ax_enums.mojom.h"
///#include "crui/accessibility/ax_node_data.h"
#include "crui/base/cursor/cursor.h"
///#include "crui/base/dragdrop/drag_drop_types.h"
///#include "crui/base/ui_base_switches_util.h"
///#include "crui/compositor/layer.h"
#include "crui/events/event.h"
#include "crui/events/event_utils.h"
#include "crui/events/keycodes/keyboard_codes.h"
///#include "crui/gfx/canvas.h"
///#include "crui/views/drag_controller.h"
#include "crui/views/layout/fill_layout.h"
#include "crui/views/view_targeter.h"
#include "crui/views/widget/root_view_targeter.h"
#include "crui/views/widget/widget.h"
#include "crui/views/widget/widget_delegate.h"
#include "crui/views/drag_controller.h"

using DispatchDetails = crui::EventDispatchDetails;

namespace crui {

namespace views {
namespace internal {

namespace {

class MouseEnterExitEvent : public crui::MouseEvent {
 public:
  MouseEnterExitEvent(const crui::MouseEvent& event, crui::EventType type)
      : crui::MouseEvent(event,
                         static_cast<View*>(nullptr),
                         static_cast<View*>(nullptr)) {
    CR_DCHECK(type == crui::ET_MOUSE_ENTERED ||
              type == crui::ET_MOUSE_EXITED);
    SetType(type);
  }

  ~MouseEnterExitEvent() override = default;
};

}  // namespace

// Used by RootView to create a hidden child that can be used to make screen
// reader announcements on platforms that don't have a reliable system API call
// to do that.
//
// We use a separate view because the RootView itself supplies the widget's
// accessible name and cannot serve double-duty (the inability for views to make
// their own announcements without changing their accessible name or description
// is the reason this system exists at all).
class AnnounceTextView : public View {
 public:
  ~AnnounceTextView() override = default;

  void Announce(const cr::string16& text) {
    // TODO(crbug.com/1024898): Use kLiveRegionChanged when supported across
    // screen readers and platforms. See bug for details.
    announce_text_ = text;
    ///NotifyAccessibilityEvent(ax::mojom::Event::kAlert, true);
  }

  // View:
  ///void GetAccessibleNodeData(crui::AXNodeData* node_data) override {
  ///  // TODO(crbug.com/1024898): Use live regions (do not use alerts).
  ///  // May require setting kLiveStatus, kContainerLiveStatus to "polite".
  ///  node_data->role = ax::mojom::Role::kAlert;
  ///  node_data->SetName(announce_text_);
  ///}

 private:
  cr::string16 announce_text_;
};

// This event handler receives events in the pre-target phase and takes care of
// the following:
//   - Shows keyboard-triggered context menus.
class PreEventDispatchHandler : public crui::EventHandler {
 public:
  PreEventDispatchHandler(const PreEventDispatchHandler&) = delete;
  PreEventDispatchHandler& operator=(const PreEventDispatchHandler&) = delete;

  explicit PreEventDispatchHandler(View* owner)
      : owner_(owner) {
    owner_->AddPreTargetHandler(this);
  }
  ~PreEventDispatchHandler() override { owner_->RemovePreTargetHandler(this); }

 private:
  // ui::EventHandler:
  void OnKeyEvent(crui::KeyEvent* event) override {
    CR_CHECK(crui::EP_PRETARGET == event->phase());
    if (event->handled())
      return;

    View* v = nullptr;
    if (owner_->GetFocusManager())  // Can be NULL in unittests.
      v = owner_->GetFocusManager()->GetFocusedView();
// macOS doesn't have keyboard-triggered context menus.
#if !defined(MINI_CHROMIUM_OS_MACOSX)
    // Special case to handle keyboard-triggered context menus.
    if (v && v->GetEnabled() &&
        ((event->key_code() == crui::VKEY_APPS) ||
         (event->key_code() == crui::VKEY_F10 && event->IsShiftDown()))) {
      // Clamp the menu location within the visible bounds of each ancestor view
      // to avoid showing the menu over a completely different view or window.
      gfx::Point location = v->GetKeyboardContextMenuLocation();
      for (View* parent = v->parent(); parent; parent = parent->parent()) {
        const gfx::Rect& parent_bounds = parent->GetBoundsInScreen();
        location.SetToMax(parent_bounds.origin());
        location.SetToMin(parent_bounds.bottom_right());
      }
      v->ShowContextMenu(location, crui::MENU_SOURCE_KEYBOARD);
      event->StopPropagation();
    }
#endif
  }

  View* owner_;
};

// This event handler receives events in the post-target phase and takes care of
// the following:
//   - Generates context menu, or initiates drag-and-drop, from gesture events.
class PostEventDispatchHandler : public crui::EventHandler {
 public:
  PostEventDispatchHandler(const PostEventDispatchHandler&) = delete;
  PostEventDispatchHandler& operator=(const PostEventDispatchHandler&) = delete;

  PostEventDispatchHandler()
      : touch_dnd_enabled_(/*::switches::IsTouchDragDropEnabled()*/false) {
  }
  ~PostEventDispatchHandler() override = default;

 private:
  // Overridden from ui::EventHandler:
  void OnGestureEvent(crui::GestureEvent* event) override {
    CR_DCHECK(crui::EP_POSTTARGET == event->phase());
    if (event->handled())
      return;
  
    View* target = static_cast<View*>(event->target());
    gfx::Point location = event->location();
    if (touch_dnd_enabled_ &&
        event->type() == crui::ET_GESTURE_LONG_PRESS &&
        (!target->drag_controller() ||
         target->drag_controller()->CanStartDragForView(
             target, location, location))) {
      if (target->DoDrag(*event, location,
          crui::DragDropTypes::DRAG_EVENT_SOURCE_TOUCH)) {
        event->StopPropagation();
        return;
      }
    }
  
    ///if (target->context_menu_controller() &&
    ///    (event->type() == crui::ET_GESTURE_LONG_PRESS ||
    ///     event->type() == crui::ET_GESTURE_LONG_TAP ||
    ///     event->type() == crui::ET_GESTURE_TWO_FINGER_TAP)) {
    ///  gfx::Point screen_location(location);
    ///  View::ConvertPointToScreen(target, &screen_location);
    ///  target->ShowContextMenu(screen_location, crui::MENU_SOURCE_TOUCH);
    ///  event->StopPropagation();
    ///}
  }

  bool touch_dnd_enabled_;
};

////////////////////////////////////////////////////////////////////////////////
// RootView, public:

// Creation and lifetime -------------------------------------------------------

RootView::RootView(Widget* widget)
    : widget_(widget),
      mouse_pressed_handler_(nullptr),
      mouse_move_handler_(nullptr),
      last_click_handler_(nullptr),
      explicit_mouse_handler_(false),
      last_mouse_event_flags_(0),
      last_mouse_event_x_(-1),
      last_mouse_event_y_(-1),
      gesture_handler_(nullptr),
      gesture_handler_set_before_processing_(false),
      pre_dispatch_handler_(new internal::PreEventDispatchHandler(this)),
      post_dispatch_handler_(new internal::PostEventDispatchHandler),
      focus_search_(this, false, false),
      focus_traversable_parent_(nullptr),
      focus_traversable_parent_view_(nullptr),
      event_dispatch_target_(nullptr),
      old_dispatch_target_(nullptr) {
  AddPostTargetHandler(post_dispatch_handler_.get());
  SetEventTargeter(
      std::unique_ptr<ViewTargeter>(new RootViewTargeter(this, this)));
}

RootView::~RootView() {
  // If we have children remove them explicitly so to make sure a remove
  // notification is sent for each one of them.
  RemoveAllChildViews(true);
}

// Tree operations -------------------------------------------------------------

void RootView::SetContentsView(View* contents_view) {
  CR_DCHECK(contents_view && GetWidget()->native_widget()) <<
      "Can't be called until after the native widget is created!";
  // The ContentsView must be set up _after_ the window is created so that its
  // Widget pointer is valid.
  SetLayoutManager(std::make_unique<FillLayout>());
  if (!children().empty())
    RemoveAllChildViews(true);
  AddChildView(contents_view);
}

View* RootView::GetContentsView() {
  return children().empty() ? nullptr : children().front();
}

void RootView::NotifyNativeViewHierarchyChanged() {
  PropagateNativeViewHierarchyChanged();
}

// Focus -----------------------------------------------------------------------

void RootView::SetFocusTraversableParent(FocusTraversable* focus_traversable) {
  CR_DCHECK(focus_traversable != this);
  focus_traversable_parent_ = focus_traversable;
}

void RootView::SetFocusTraversableParentView(View* view) {
  focus_traversable_parent_view_ = view;
}

// System events ---------------------------------------------------------------

void RootView::ThemeChanged() {
  View::PropagateThemeChanged();
}

void RootView::ResetEventHandlers() {
  explicit_mouse_handler_ = false;
  mouse_pressed_handler_ = nullptr;
  mouse_move_handler_ = nullptr;
  gesture_handler_ = nullptr;
  event_dispatch_target_ = nullptr;
  old_dispatch_target_ = nullptr;
}

void RootView::DeviceScaleFactorChanged(float old_device_scale_factor,
                                        float new_device_scale_factor) {
  View::PropagateDeviceScaleFactorChanged(old_device_scale_factor,
                                          new_device_scale_factor);
}

// Accessibility ---------------------------------------------------------------

void RootView::AnnounceText(const cr::string16& text) {
#if defined(MINI_CHROMIUM_OS_MACOSX)
  // MacOSX has its own API for making announcements; see AnnounceText()
  // override in ax_platform_node_mac.[h|mm]
  CR_NOTREACHED();
#else
  CR_DCHECK(GetWidget());
  CR_DCHECK(GetContentsView());
  if (!announce_view_) {
    announce_view_ = AddChildView(std::make_unique<AnnounceTextView>());
    static_cast<FillLayout*>(GetLayoutManager())
        ->SetChildViewIgnoredByLayout(announce_view_, true);
  }
  announce_view_->Announce(text);
#endif
}

////////////////////////////////////////////////////////////////////////////////
// RootView, FocusTraversable implementation:

FocusSearch* RootView::GetFocusSearch() {
  return &focus_search_;
}

FocusTraversable* RootView::GetFocusTraversableParent() {
  return focus_traversable_parent_;
}

View* RootView::GetFocusTraversableParentView() {
  return focus_traversable_parent_view_;
}

////////////////////////////////////////////////////////////////////////////////
// RootView, ui::EventProcessor overrides:

crui::EventTarget* RootView::GetRootForEvent(crui::Event* event) {
  return this;
}

crui::EventTargeter* RootView::GetDefaultEventTargeter() {
  return this->GetEventTargeter();
}

void RootView::OnEventProcessingStarted(crui::Event* event) {
  ///if (!event->IsGestureEvent())
  ///  return;
  ///
  ///crui::GestureEvent* gesture_event = event->AsGestureEvent();
  ///
  ///// Do not process ui::ET_GESTURE_BEGIN events.
  ///if (gesture_event->type() == ui::ET_GESTURE_BEGIN) {
  ///  event->SetHandled();
  ///  return;
  ///}
  ///
  ///// Do not process ui::ET_GESTURE_END events if they do not correspond to the
  ///// removal of the final touch point or if no gesture handler has already
  ///// been set.
  ///if (gesture_event->type() == ui::ET_GESTURE_END &&
  ///    (gesture_event->details().touch_points() > 1 ||
  ///     !gesture_handler_)) {
  ///  event->SetHandled();
  ///  return;
  ///}
  ///
  ///// Do not process subsequent gesture scroll events if no handler was set for
  ///// a ui::ET_GESTURE_SCROLL_BEGIN event.
  ///if (!gesture_handler_ &&
  ///    (gesture_event->type() == ui::ET_GESTURE_SCROLL_UPDATE ||
  ///     gesture_event->type() == ui::ET_GESTURE_SCROLL_END ||
  ///     gesture_event->type() == ui::ET_SCROLL_FLING_START)) {
  ///  event->SetHandled();
  ///  return;
  ///}
  ///
  ///gesture_handler_set_before_processing_ = !!gesture_handler_;
}

void RootView::OnEventProcessingFinished(crui::Event* event) {
  // If |event| was not handled and |gesture_handler_| was not set by the
  // dispatch of a previous gesture event, then no default gesture handler
  // should be set prior to the next gesture event being received.
  ///if (event->IsGestureEvent() &&
  ///    !event->handled() &&
  ///    !gesture_handler_set_before_processing_) {
  ///  gesture_handler_ = nullptr;
  ///}
}

////////////////////////////////////////////////////////////////////////////////
// RootView, View overrides:

bool RootView::IsDrawn() const {
  return GetVisible();
}

bool RootView::OnMousePressed(const crui::MouseEvent& event) {
  UpdateCursor(event);
  SetMouseLocationAndFlags(event);

  // If mouse_pressed_handler_ is non null, we are currently processing
  // a pressed -> drag -> released session. In that case we send the
  // event to mouse_pressed_handler_
  if (mouse_pressed_handler_) {
    crui::MouseEvent mouse_pressed_event(event, static_cast<View*>(this),
                                         mouse_pressed_handler_);
    drag_info_.Reset();
    crui::EventDispatchDetails dispatch_details =
        DispatchEvent(mouse_pressed_handler_, &mouse_pressed_event);
    if (dispatch_details.dispatcher_destroyed)
      return true;
    return true;
  }
  CR_DCHECK(!explicit_mouse_handler_);

  bool hit_disabled_view = false;
  // Walk up the tree until we find a view that wants the mouse event.
  for (mouse_pressed_handler_ = GetEventHandlerForPoint(event.location());
       mouse_pressed_handler_ && (mouse_pressed_handler_ != this);
       mouse_pressed_handler_ = mouse_pressed_handler_->parent()) {
    CR_DVLOG(1) << "OnMousePressed testing "
        << mouse_pressed_handler_->GetClassName();
    if (!mouse_pressed_handler_->GetEnabled()) {
      // Disabled views should eat events instead of propagating them upwards.
      hit_disabled_view = true;
      break;
    }

    // See if this view wants to handle the mouse press.
    crui::MouseEvent mouse_pressed_event(event, static_cast<View*>(this),
                                         mouse_pressed_handler_);

    // Remove the double-click flag if the handler is different than the
    // one which got the first click part of the double-click.
    if (mouse_pressed_handler_ != last_click_handler_)
      mouse_pressed_event.set_flags(event.flags() & ~crui::EF_IS_DOUBLE_CLICK);

    drag_info_.Reset();
    crui::EventDispatchDetails dispatch_details =
        DispatchEvent(mouse_pressed_handler_, &mouse_pressed_event);
    if (dispatch_details.dispatcher_destroyed)
      return mouse_pressed_event.handled();

    // The view could have removed itself from the tree when handling
    // OnMousePressed().  In this case, the removal notification will have
    // reset mouse_pressed_handler_ to NULL out from under us.  Detect this
    // case and stop.  (See comments in view.h.)
    //
    // NOTE: Don't return true here, because we don't want the frame to
    // forward future events to us when there's no handler.
    if (!mouse_pressed_handler_)
      break;

    // If the view handled the event, leave mouse_pressed_handler_ set and
    // return true, which will cause subsequent drag/release events to get
    // forwarded to that view.
    if (mouse_pressed_event.handled()) {
      last_click_handler_ = mouse_pressed_handler_;
      CR_DVLOG(1) << "OnMousePressed handled by "
          << mouse_pressed_handler_->GetClassName();
      return true;
    }
  }

  // Reset mouse_pressed_handler_ to indicate that no processing is occurring.
  mouse_pressed_handler_ = nullptr;

  // In the event that a double-click is not handled after traversing the
  // entire hierarchy (even as a single-click when sent to a different view),
  // it must be marked as handled to avoid anything happening from default
  // processing if it the first click-part was handled by us.
  if (last_click_handler_ && (event.flags() & crui::EF_IS_DOUBLE_CLICK))
    hit_disabled_view = true;

  last_click_handler_ = nullptr;
  return hit_disabled_view;
}

bool RootView::OnMouseDragged(const crui::MouseEvent& event) {
  if (mouse_pressed_handler_) {
    SetMouseLocationAndFlags(event);

    crui::MouseEvent mouse_event(event, static_cast<View*>(this),
                                 mouse_pressed_handler_);
    crui::EventDispatchDetails dispatch_details =
        DispatchEvent(mouse_pressed_handler_, &mouse_event);
    if (dispatch_details.dispatcher_destroyed)
      return false;
  }
  return false;
}

void RootView::OnMouseReleased(const crui::MouseEvent& event) {
  UpdateCursor(event);

  if (mouse_pressed_handler_) {
    crui::MouseEvent mouse_released(event, static_cast<View*>(this),
                                    mouse_pressed_handler_);
    // We allow the view to delete us from the event dispatch callback. As such,
    // configure state such that we're done first, then call View.
    View* mouse_pressed_handler = mouse_pressed_handler_;
    SetMouseHandler(nullptr);
    crui::EventDispatchDetails dispatch_details =
        DispatchEvent(mouse_pressed_handler, &mouse_released);
    if (dispatch_details.dispatcher_destroyed)
      return;
  }
}

void RootView::OnMouseCaptureLost() {
  // TODO: this likely needs to reset touch handler too.

  if (mouse_pressed_handler_ || gesture_handler_) {
    // Synthesize a release event for UpdateCursor.
    if (mouse_pressed_handler_) {
      gfx::Point last_point(last_mouse_event_x_, last_mouse_event_y_);
      crui::MouseEvent release_event(crui::ET_MOUSE_RELEASED, last_point,
                                     last_point, crui::EventTimeForNow(),
                                     last_mouse_event_flags_, 0);
      UpdateCursor(release_event);
    }
    // We allow the view to delete us from OnMouseCaptureLost. As such,
    // configure state such that we're done first, then call View.
    View* mouse_pressed_handler = mouse_pressed_handler_;
    View* gesture_handler = gesture_handler_;
    SetMouseHandler(nullptr);
    if (mouse_pressed_handler)
      mouse_pressed_handler->OnMouseCaptureLost();
    else
      gesture_handler->OnMouseCaptureLost();
    // WARNING: we may have been deleted.
  }
}

void RootView::OnMouseMoved(const crui::MouseEvent& event) {
  View* v = GetEventHandlerForPoint(event.location());
  // Find the first enabled view, or the existing move handler, whichever comes
  // first.  The check for the existing handler is because if a view becomes
  // disabled while handling moves, it's wrong to suddenly send ET_MOUSE_EXITED
  // and ET_MOUSE_ENTERED events, because the mouse hasn't actually exited yet.
  while (v && !v->GetEnabled() && (v != mouse_move_handler_))
    v = v->parent();
  if (v && v != this) {
    if (v != mouse_move_handler_) {
      if (mouse_move_handler_ != nullptr &&
          (!mouse_move_handler_->notify_enter_exit_on_child() ||
           !mouse_move_handler_->Contains(v))) {
        MouseEnterExitEvent exit(event, crui::ET_MOUSE_EXITED);
        exit.ConvertLocationToTarget(static_cast<View*>(this),
                                     mouse_move_handler_);
        crui::EventDispatchDetails dispatch_details =
            DispatchEvent(mouse_move_handler_, &exit);
        if (dispatch_details.dispatcher_destroyed)
          return;
        // The mouse_move_handler_ could have been destroyed in the context of
        // the mouse exit event.
        if (!dispatch_details.target_destroyed) {
          // View was removed by ET_MOUSE_EXITED, or |mouse_move_handler_| was
          // cleared, perhaps by a nested event handler, so return and wait for
          // the next mouse move event.
          if (!mouse_move_handler_)
            return;
          dispatch_details = NotifyEnterExitOfDescendant(
              event, crui::ET_MOUSE_EXITED, mouse_move_handler_, v);
          if (dispatch_details.dispatcher_destroyed)
            return;
        }
      }
      View* old_handler = mouse_move_handler_;
      mouse_move_handler_ = v;
      if (!mouse_move_handler_->notify_enter_exit_on_child() ||
          !mouse_move_handler_->Contains(old_handler)) {
        MouseEnterExitEvent entered(event, crui::ET_MOUSE_ENTERED);
        entered.ConvertLocationToTarget(static_cast<View*>(this),
                                        mouse_move_handler_);
        crui::EventDispatchDetails dispatch_details =
            DispatchEvent(mouse_move_handler_, &entered);
        if (dispatch_details.dispatcher_destroyed ||
            dispatch_details.target_destroyed) {
          return;
        }
        // View was removed by ET_MOUSE_ENTERED, or |mouse_move_handler_| was
        // cleared, perhaps by a nested event handler, so return and wait for
        // the next mouse move event.
        if (!mouse_move_handler_)
          return;
        dispatch_details = NotifyEnterExitOfDescendant(
            event, crui::ET_MOUSE_ENTERED, mouse_move_handler_, old_handler);
        if (dispatch_details.dispatcher_destroyed ||
            dispatch_details.target_destroyed) {
          return;
        }
      }
    }
    crui::MouseEvent moved_event(event, static_cast<View*>(this),
                                 mouse_move_handler_);
    mouse_move_handler_->OnMouseMoved(moved_event);
    // TODO(tdanderson): It may be possible to avoid setting the cursor twice
    //                   (once here and once from CompoundEventFilter) on a
    //                   mousemove. See crbug.com/351469.
    if (!(moved_event.flags() & crui::EF_IS_NON_CLIENT))
      widget_->SetCursor(mouse_move_handler_->GetCursor(moved_event));
  } else if (mouse_move_handler_ != nullptr) {
    MouseEnterExitEvent exited(event, crui::ET_MOUSE_EXITED);
    crui::EventDispatchDetails dispatch_details =
        DispatchEvent(mouse_move_handler_, &exited);
    if (dispatch_details.dispatcher_destroyed)
      return;
    // The mouse_move_handler_ could have been destroyed in the context of the
    // mouse exit event.
    if (!dispatch_details.target_destroyed) {
      // View was removed by ET_MOUSE_EXITED, or |mouse_move_handler_| was
      // cleared, perhaps by a nested event handler, so return and wait for
      // the next mouse move event.
      if (!mouse_move_handler_)
        return;
      dispatch_details = NotifyEnterExitOfDescendant(
          event, crui::ET_MOUSE_EXITED, mouse_move_handler_, v);
      if (dispatch_details.dispatcher_destroyed)
        return;
    }
    // On Aura the non-client area extends slightly outside the root view for
    // some windows.  Let the non-client cursor handling code set the cursor
    // as we do above.
    if (!(event.flags() & crui::EF_IS_NON_CLIENT))
      widget_->SetCursor(gfx::kNullCursor);
    mouse_move_handler_ = nullptr;
  }
}

void RootView::OnMouseExited(const crui::MouseEvent& event) {
  if (mouse_move_handler_ != nullptr) {
    MouseEnterExitEvent exited(event, crui::ET_MOUSE_EXITED);
    crui::EventDispatchDetails dispatch_details =
        DispatchEvent(mouse_move_handler_, &exited);
    if (dispatch_details.dispatcher_destroyed)
      return;
    // The mouse_move_handler_ could have been destroyed in the context of the
    // mouse exit event.
    if (!dispatch_details.target_destroyed) {
      CR_CHECK(mouse_move_handler_);
      dispatch_details = NotifyEnterExitOfDescendant(
          event, crui::ET_MOUSE_EXITED, mouse_move_handler_, nullptr);
      if (dispatch_details.dispatcher_destroyed)
        return;
    }
    mouse_move_handler_ = nullptr;
  }
}

bool RootView::OnMouseWheel(const crui::MouseWheelEvent& event) {
  for (View* v = GetEventHandlerForPoint(event.location());
       v && v != this && !event.handled(); v = v->parent()) {
    crui::EventDispatchDetails dispatch_details =
        DispatchEvent(v, const_cast<crui::MouseWheelEvent*>(&event));
    if (dispatch_details.dispatcher_destroyed ||
        dispatch_details.target_destroyed) {
      return event.handled();
    }
  }
  return event.handled();
}

void RootView::SetMouseHandler(View* new_mh) {
  // If we're clearing the mouse handler, clear explicit_mouse_handler_ as well.
  explicit_mouse_handler_ = (new_mh != nullptr);
  mouse_pressed_handler_ = new_mh;
  gesture_handler_ = new_mh;
  drag_info_.Reset();
}

///void RootView::GetAccessibleNodeData(crui::AXNodeData* node_data) {
///  DCHECK(GetWidget());
///  auto* widget_delegate = GetWidget()->widget_delegate();
///  if (!widget_delegate)
///    return;
///  node_data->SetName(widget_delegate->GetAccessibleWindowTitle());
///  node_data->role = widget_delegate->GetAccessibleWindowRole();
///}

void RootView::UpdateParentLayer() {
  ///if (layer())
  ///  ReparentLayer(widget_->GetLayer());
}

////////////////////////////////////////////////////////////////////////////////
// RootView, protected:

void RootView::ViewHierarchyChanged(
    const ViewHierarchyChangedDetails& details) {
  widget_->ViewHierarchyChanged(details);

  if (!details.is_add) {
    if (!explicit_mouse_handler_ && mouse_pressed_handler_ == details.child)
      mouse_pressed_handler_ = nullptr;
    if (mouse_move_handler_ == details.child)
      mouse_move_handler_ = nullptr;
    if (gesture_handler_ == details.child)
      gesture_handler_ = nullptr;
    if (event_dispatch_target_ == details.child)
      event_dispatch_target_ = nullptr;
    if (old_dispatch_target_ == details.child)
      old_dispatch_target_ = nullptr;
  }
}

void RootView::VisibilityChanged(View* /*starting_from*/, bool is_visible) {
  if (!is_visible) {
    // When the root view is being hidden (e.g. when widget is minimized)
    // handlers are reset, so that after it is reshown, events are not captured
    // by old handlers.
    ResetEventHandlers();
  }
}

void RootView::OnDidSchedulePaint(const gfx::Rect& rect) {
  ///if (!layer()) {
    gfx::Rect xrect = ConvertRectToParent(rect);
    gfx::Rect invalid_rect = gfx::IntersectRects(GetLocalBounds(), xrect);
    if (!invalid_rect.IsEmpty())
      widget_->SchedulePaintInRect(invalid_rect);
  ///}
}

///void RootView::OnPaint(gfx::Canvas* canvas) {
///  if (!layer() || !layer()->fills_bounds_opaquely())
///    canvas->DrawColor(SK_ColorBLACK, SkBlendMode::kClear);
///
///  View::OnPaint(canvas);
///}

///View::LayerOffsetData RootView::CalculateOffsetToAncestorWithLayer(
///    ui::Layer** layer_parent) {
///  if (layer() || !widget_->GetLayer())
///    return View::CalculateOffsetToAncestorWithLayer(layer_parent);
///  if (layer_parent)
///    *layer_parent = widget_->GetLayer();
///  return LayerOffsetData(widget_->GetLayer()->device_scale_factor());
///}

View::DragInfo* RootView::GetDragInfo() {
  return &drag_info_;
}

////////////////////////////////////////////////////////////////////////////////
// RootView, private:

void RootView::UpdateCursor(const crui::MouseEvent& event) {
  if (!(event.flags() & crui::EF_IS_NON_CLIENT)) {
    View* v = GetEventHandlerForPoint(event.location());
    crui::MouseEvent me(event, static_cast<View*>(this), v);
    widget_->SetCursor(v->GetCursor(me));
  }
}

void RootView::SetMouseLocationAndFlags(const crui::MouseEvent& event) {
  last_mouse_event_flags_ = event.flags();
  last_mouse_event_x_ = static_cast<int>(event.x());
  last_mouse_event_y_ = static_cast<int>(event.y());
}

crui::EventDispatchDetails RootView::NotifyEnterExitOfDescendant(
    const crui::MouseEvent& event,
    crui::EventType type,
    View* view,
    View* sibling) {
  for (View* p = view->parent(); p; p = p->parent()) {
    if (!p->notify_enter_exit_on_child())
      continue;
    if (sibling && p->Contains(sibling))
      break;
    // It is necessary to recreate the notify-event for each dispatch, since one
    // of the callbacks can mark the event as handled, and that would cause
    // incorrect event dispatch.
    MouseEnterExitEvent notify_event(event, type);
    crui::EventDispatchDetails dispatch_details 
        = DispatchEvent(p, &notify_event);
    if (dispatch_details.dispatcher_destroyed ||
        dispatch_details.target_destroyed) {
      return dispatch_details;
    }
  }
  return crui::EventDispatchDetails();
}

bool RootView::CanDispatchToTarget(crui::EventTarget* target) {
  return event_dispatch_target_ == target;
}

crui::EventDispatchDetails RootView::PreDispatchEvent(
    crui::EventTarget* target,
    crui::Event* event) {
  View* view = static_cast<View*>(target);
  if (event->IsGestureEvent()) {
    // Update |gesture_handler_| to indicate which View is currently handling
    // gesture events.
    // TODO(tdanderson): Look into moving this to PostDispatchEvent() and
    //                   using |event_dispatch_target_| instead of
    //                   |gesture_handler_| to detect if the view has been
    //                   removed from the tree.
    gesture_handler_ = view;

    // Disabled views are permitted to be targets of gesture events, but
    // gesture events should never actually be dispatched to them. Prevent
    // dispatch by marking the event as handled.
    if (!view->GetEnabled())
      event->SetHandled();
  }

  old_dispatch_target_ = event_dispatch_target_;
  event_dispatch_target_ = view;
  return DispatchDetails();
}

crui::EventDispatchDetails RootView::PostDispatchEvent(
    crui::EventTarget* target,
    const crui::Event& event) {
  // The GESTURE_END event corresponding to the removal of the final touch
  // point marks the end of a gesture sequence, so reset |gesture_handler_|
  // to NULL.
  if (event.type() == crui::ET_GESTURE_END) {
    // In case a drag was in progress, reset all the handlers. Otherwise, just
    // reset the gesture handler.
    if (gesture_handler_ && gesture_handler_ == mouse_pressed_handler_)
      SetMouseHandler(nullptr);
    else
      gesture_handler_ = nullptr;
  }

  DispatchDetails details;
  if (target != event_dispatch_target_)
    details.target_destroyed = true;

  event_dispatch_target_ = old_dispatch_target_;
  old_dispatch_target_ = nullptr;

#ifndef NDEBUG
  CR_DCHECK(!event_dispatch_target_ || Contains(event_dispatch_target_));
#endif

  return details;
}

const Widget* RootView::GetWidgetImpl() const {
  return widget_;
}

BEGIN_METADATA(RootView)
METADATA_PARENT_CLASS(View)
END_METADATA()
}  // namespace internal
}  // namespace views
}  // namespace crui
