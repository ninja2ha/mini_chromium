// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/view.h"

#include <algorithm>
#include <memory>
#include <utility>

///#include "crbase/command_line.h"
#include "crbase/containers/adapters.h"
///#include "crbase/feature_list.h"
#include "crbase/logging.h"
#include "crbase/helper/stl_util.h"
#include "crbase/strings/stringprintf.h"
#include "crbase/strings/utf_string_conversions.h"
///#include "crbase/trace_event/trace_event.h"
///#include "third_party/skia/include/core/SkRect.h"
///#include "crui/accessibility/ax_action_data.h"
///#include "crui/accessibility/ax_enums.mojom.h"
#include "crui/base/i18n/rtl.h"
#include "crui/base/cursor/cursor.h"
#include "crui/base/dragdrop/drag_drop_types.h"
///#include "crui/base/ime/input_method.h"
///#include "crui/compositor/clip_recorder.h"
///#include "crui/compositor/compositor.h"
///#include "crui/compositor/layer.h"
///#include "crui/compositor/layer_animator.h"
///#include "crui/compositor/paint_context.h"
///#include "crui/compositor/paint_recorder.h"
///#include "crui/compositor/transform_recorder.h"
#include "crui/display/screen.h"
#include "crui/events/base_event_utils.h"
#include "crui/events/event_target_iterator.h"
#include "crui/gfx/canvas.h"
#include "crui/gfx/scoped_canvas.h"
#include "crui/gfx/geometry/angle_conversions.h"
#include "crui/gfx/geometry/point3_f.h"
#include "crui/gfx/geometry/point_conversions.h"
///#include "crui/gfx/interpolated_transform.h"
///#include "crui/gfx/scoped_canvas.h"
///#include "crui/gfx/skia_util.h"
#include "crui/gfx/geometry/transform.h"
///#include "crui/native_theme/native_theme.h"
///#include "crui/views/accessibility/ax_event_manager.h"
///#include "crui/views/accessibility/view_accessibility.h"
#include "crui/views/view_background.h"
#include "crui/views/view_border.h"
///#include "crui/views/buildflags.h"
///#include "crui/views/context_menu_controller.h"
#include "crui/views/drag_controller.h"
#include "crui/views/layout/layout_manager.h"
#include "crui/views/metadata/metadata_impl_macros.h"
#include "crui/views/view_observer.h"
#include "crui/views/view_tracker.h"
///#include "crui/views/views_features.h"
///#include "crui/views/views_switches.h"
#include "crui/views/widget/native_widget_private.h"
#include "crui/views/widget/root_view.h"
#include "crui/views/widget/tooltip_manager.h"
#include "crui/views/widget/widget.h"
#include "crui/base/ui_export.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include "crui/base/win/scoped_gdi_object.h"
///#include "crui/native_theme/native_theme_win.h"
#endif

namespace crui {
namespace views {

namespace {

#if defined(MINI_CHROMIUM_OS_WIN)
constexpr bool kContextMenuOnMousePress = false;
#else
constexpr bool kContextMenuOnMousePress = true;
#endif

// Default horizontal drag threshold in pixels.
// Same as what gtk uses.
constexpr int kDefaultHorizontalDragThreshold = 8;

// Default vertical drag threshold in pixels.
// Same as what gtk uses.
constexpr int kDefaultVerticalDragThreshold = 8;

// Returns the top view in |view|'s hierarchy.
const View* GetHierarchyRoot(const View* view) {
  const View* root = view;
  while (root && root->parent())
    root = root->parent();
  return root;
}

}  // namespace

namespace internal {

#if CR_DCHECK_IS_ON()
class ScopedChildrenLock {
 public:
  ScopedChildrenLock(const ScopedChildrenLock&) = delete;
  ScopedChildrenLock& operator=(const ScopedChildrenLock&) = delete;

  explicit ScopedChildrenLock(const View* view)
      : reset_(&view->iterating_, true) {}
  ~ScopedChildrenLock() = default;

 private:
  cr::AutoReset<bool> reset_;
};
#else
class ScopedChildrenLock {
 public:
  explicit ScopedChildrenLock(const View* view) {}
  ~ScopedChildrenLock() {}
};
#endif

}  // namespace internal

////////////////////////////////////////////////////////////////////////////////
// View, public:

// Creation and lifetime -------------------------------------------------------

View::View() {
  SetTargetHandler(this);
}

View::~View() {
  if (parent_)
    parent_->RemoveChildView(this);

  {
    internal::ScopedChildrenLock lock(this);
    for (auto* child : children_) {
      child->parent_ = nullptr;
      if (!child->owned_by_client_)
        delete child;
    }
  }

  for (ViewObserver& observer : observers_)
    observer.OnViewIsDeleting(this);

  for (crui::Layer* layer_beneath : layers_beneath_)
    layer_beneath->RemoveObserver(this);

  // Clearing properties explicitly here lets us guarantee that properties
  // outlive |this| (at least the View part of |this|). This is intentionally
  // called at the end so observers can examine properties inside
  // OnViewIsDeleting(), for instance.
  ClearProperties();
}

// Tree operations -------------------------------------------------------------

const Widget* View::GetWidgetImpl() const {
  // The root view holds a reference to this view hierarchy's Widget.
  return parent_ ? parent_->GetWidget() : nullptr;
}

void View::ReorderChildView(View* view, int index) {
  CR_DCHECK(view->parent_ == this);
  const auto i = std::find(children_.begin(), children_.end(), view);
  CR_DCHECK(i != children_.end());

  // If |view| is already at the desired position, there's nothing to do.
  const bool move_to_end = (index < 0) || 
                           (static_cast<size_t>(index) >= children_.size());
  const auto pos = move_to_end ? std::prev(children_.end())
                               : std::next(children_.begin(), index);
  if (i == pos)
    return;

    // Rotate |view| to be at the desired position.
#if CR_DCHECK_IS_ON()
  CR_DCHECK(!iterating_);
#endif
  if (pos < i)
    std::rotate(pos, i, std::next(i));
  else
    std::rotate(i, std::next(i), std::next(pos));

  // Update focus siblings.  Unhook |view| from the focus cycle first so
  // SetFocusSiblings() won't traverse through it.
  View* next_focusable = view->next_focusable_view_;
  View* prev_focusable = view->previous_focusable_view_;
  if (prev_focusable)
    prev_focusable->next_focusable_view_ = next_focusable;
  if (next_focusable)
    next_focusable->previous_focusable_view_ = prev_focusable;
  SetFocusSiblings(view, pos);

  for (ViewObserver& observer : observers_)
    observer.OnChildViewReordered(this, view);

  ReorderLayers();
  InvalidateLayout();
}

void View::RemoveChildView(View* view) {
  DoRemoveChildView(view, true, true, false, nullptr);
}

void View::RemoveAllChildViews(bool delete_children) {
  while (!children_.empty())
    DoRemoveChildView(children_.front(), false, false, delete_children,
                      nullptr);
  UpdateTooltip();
}

bool View::Contains(const View* view) const {
  for (const View* v = view; v; v = v->parent_) {
    if (v == this)
      return true;
  }
  return false;
}

View::Views::const_iterator View::FindChild(const View* view) const {
  return std::find(children_.cbegin(), children_.cend(), view);
}

int View::GetIndexOf(const View* view) const {
  const auto i = FindChild(view);
  return i == children_.cend()
             ? -1
             : static_cast<int>(std::distance(children_.cbegin(), i));
}

// Size and disposition --------------------------------------------------------

void View::SetBounds(int x, int y, int width, int height) {
  SetBoundsRect(gfx::Rect(x, y, std::max(0, width), std::max(0, height)));
}

void View::SetBoundsRect(const gfx::Rect& bounds) {
  if (bounds == bounds_) {
    if (needs_layout_) {
      needs_layout_ = false;
      ///TRACE_EVENT1("views", "View::Layout(set_bounds)", "class",
      ///             GetClassName());
      Layout();
    }
    return;
  }

  bool is_size_changed = bounds_.size() != bounds.size();
  // Paint where the view is currently.
  SchedulePaintBoundsChanged(is_size_changed);

  gfx::Rect prev = bounds_;
  bounds_ = bounds;

  // Paint the new bounds.
  SchedulePaintBoundsChanged(is_size_changed);

  if (layer()) {
    if (parent_) {
      LayerOffsetData offset_data(
          parent_->CalculateOffsetToAncestorWithLayer(nullptr));
      offset_data += GetMirroredPosition().OffsetFromOrigin();
      SetLayerBounds(size(), offset_data);
    } else {
      SetLayerBounds(bounds_.size(),
                     LayerOffsetData() + bounds_.OffsetFromOrigin());
    }
  
    // In RTL mode, if our width has changed, our children's mirrored bounds
    // will have changed. Update the child's layer bounds, or if it is not a
    // layer, the bounds of any layers inside the child.
    if (crui::i18n::IsRTL() && bounds_.width() != prev.width()) {
      for (View* child : children_) {
        child->UpdateChildLayerBounds(
            LayerOffsetData(layer()->device_scale_factor(),
                            child->GetMirroredPosition().OffsetFromOrigin()));
      }
    }
  } else {
    // If our bounds have changed, then any descendant layer bounds may have
    // changed. Update them accordingly.
    UpdateChildLayerBounds(CalculateOffsetToAncestorWithLayer(nullptr));
  }

  OnBoundsChanged(prev);
  ///if (bounds_ != prev)
  ///  NotifyAccessibilityEvent(ax::mojom::Event::kLocationChanged, false);

  if (needs_layout_ || is_size_changed) {
    needs_layout_ = false;
    ///TRACE_EVENT1("views", "View::Layout(bounds_changed)", "class",
    ///             GetClassName());
    Layout();
  }

  if (GetNeedsNotificationWhenVisibleBoundsChange())
    OnVisibleBoundsChanged();

  // Notify interested Views that visible bounds within the root view may have
  // changed.
  if (descendants_to_notify_) {
    for (auto* i : *descendants_to_notify_) {
      i->OnVisibleBoundsChanged();
    }
  }

  for (ViewObserver& observer : observers_)
    observer.OnViewBoundsChanged(this);
}

void View::SetSize(const gfx::Size& size) {
  SetBounds(x(), y(), size.width(), size.height());
}

void View::SetPosition(const gfx::Point& position) {
  SetBounds(position.x(), position.y(), width(), height());
}

void View::SetX(int x) {
  SetBounds(x, y(), width(), height());
}

void View::SetY(int y) {
  SetBounds(x(), y, width(), height());
}

gfx::Rect View::GetContentsBounds() const {
  gfx::Rect contents_bounds(GetLocalBounds());
  contents_bounds.Inset(GetInsets());
  return contents_bounds;
}

gfx::Rect View::GetLocalBounds() const {
  return gfx::Rect(size());
}

gfx::Insets View::GetInsets() const {
  ///return border_ ? border_->GetInsets() : gfx::Insets();
  return gfx::Insets();
}

gfx::Rect View::GetVisibleBounds() const {
  if (!IsDrawn())
    return gfx::Rect();

  gfx::Rect vis_bounds(GetLocalBounds());
  gfx::Rect ancestor_bounds;
  const View* view = this;
  gfx::Transform transform;

  while (view != nullptr && !vis_bounds.IsEmpty()) {
    transform.PostConcat(view->GetTransform());
    gfx::Transform translation;
    translation.Translate(static_cast<float>(view->GetMirroredX()),
                          static_cast<float>(view->y()));
    transform.PostConcat(translation);

    vis_bounds = view->ConvertRectToParent(vis_bounds);
    const View* ancestor = view->parent_;
    if (ancestor != nullptr) {
      ancestor_bounds.SetRect(0, 0, ancestor->width(), ancestor->height());
      vis_bounds.Intersect(ancestor_bounds);
    } else if (!view->GetWidget()) {
      // If the view has no Widget, we're not visible. Return an empty rect.
      return gfx::Rect();
    }
    view = ancestor;
  }
  if (vis_bounds.IsEmpty())
    return vis_bounds;

  // Convert back to this views coordinate system. This mapping returns the
  // enclosing rect, which is good because partially visible pixels should
  // be considered visible.
  return transform.InverseMapRect(vis_bounds).value_or(vis_bounds);
}

gfx::Rect View::GetBoundsInScreen() const {
  gfx::Point origin;
  View::ConvertPointToScreen(this, &origin);
  return gfx::Rect(origin, size());
}

gfx::Rect View::GetAnchorBoundsInScreen() const {
  return GetBoundsInScreen();
}

gfx::Size View::GetPreferredSize() const {
  if (preferred_size_)
    return *preferred_size_;
  return CalculatePreferredSize();
}

int View::GetBaseline() const {
  return -1;
}

void View::SetPreferredSize(const gfx::Size& size) {
  if (preferred_size_ && *preferred_size_ == size)
    return;

  preferred_size_ = size;
  PreferredSizeChanged();
}

void View::SizeToPreferredSize() {
  SetSize(GetPreferredSize());
}

gfx::Size View::GetMinimumSize() const {
  if (layout_manager_)
    return layout_manager_->GetMinimumSize(this);

  return GetPreferredSize();
}

gfx::Size View::GetMaximumSize() const {
  return gfx::Size();
}

int View::GetHeightForWidth(int w) const {
  if (layout_manager_)
    return layout_manager_->GetPreferredHeightForWidth(this, w);
  return GetPreferredSize().height();
}

SizeBounds View::GetAvailableSize(const View* child) const {
  if (layout_manager_)
    return layout_manager_->GetAvailableSize(this, child);
  return SizeBounds();
}

bool View::GetVisible() const {
  return visible_;
}

void View::SetVisible(bool visible) {
  const bool was_visible = visible_;
  if (was_visible != visible) {
    // If the View was visible, schedule paint to refresh parent.
    // TODO(beng): not sure we should be doing this if we have a layer.
    if (was_visible)
      SchedulePaint();

    visible_ = visible;
    AdvanceFocusIfNecessary();

    // Notify the parent.
    if (parent_) {
      parent_->ChildVisibilityChanged(this);
      ///parent_->NotifyAccessibilityEvent(ax::mojom::Event::kChildrenChanged,
       ///                                 true);
    }

    // This notifies all sub-views recursively.
    PropagateVisibilityNotifications(this, visible_);
    UpdateLayerVisibility();

    // Notify all other subscriptions of the change.
    OnPropertyChanged(&visible_, kPropertyEffectsPaint);
  }

  if (parent_) {
    LayoutManager* const layout_manager = parent_->GetLayoutManager();
    if (layout_manager && layout_manager->view_setting_visibility_on_ != this)
      layout_manager->ViewVisibilitySet(parent_, this, was_visible, visible);
  }
}

PropertyChangedSubscription View::AddVisibleChangedCallback(
    PropertyChangedCallback callback) {
  return AddPropertyChangedCallback(&visible_, std::move(callback));
}

bool View::IsDrawn() const {
  return visible_ && parent_ ? parent_->IsDrawn() : false;
}

bool View::GetEnabled() const {
  return enabled_;
}

void View::SetEnabled(bool enabled) {
  if (enabled_ == enabled)
    return;

  enabled_ = enabled;
  AdvanceFocusIfNecessary();
  ///NotifyAccessibilityEvent(ax::mojom::Event::kStateChanged, true);
  OnPropertyChanged(&enabled_, kPropertyEffectsPaint);
}

PropertyChangedSubscription View::AddEnabledChangedCallback(
    PropertyChangedCallback callback) {
  return AddPropertyChangedCallback(&enabled_, std::move(callback));
}

View::Views View::GetChildrenInZOrder() {
  if (layout_manager_) {
    const auto result = layout_manager_->GetChildViewsInPaintOrder(this);
    CR_DCHECK(children_.size() == result.size());
    return result;
  }
  return children_;
}

// Transformations -------------------------------------------------------------

gfx::Transform View::GetTransform() const {
  if (!layer())
    return gfx::Transform();
  
  gfx::Transform transform = layer()->transform();
  ///gfx::ScrollOffset scroll_offset = layer()->CurrentScrollOffset();
  // Offsets for layer-based scrolling are never negative, but the horizontal
  // scroll direction is reversed in RTL via canvas flipping.
  ///transform.Translate((cr::i18n::IsRTL() ? 1 : -1) * scroll_offset.x(),
  ///                    -scroll_offset.y());
  return transform;
}

void View::SetTransform(const gfx::Transform& transform) {
  if (transform.IsIdentity()) {
    if (layer())
      layer()->SetTransform(transform);
    paint_to_layer_for_transform_ = false;
    CreateOrDestroyLayer();
  } else {
    paint_to_layer_for_transform_ = true;
    CreateOrDestroyLayer();
    CR_DCHECK(layer() != nullptr);
    layer()->SetTransform(transform);
    ///layer()->ScheduleDraw();
  }
  
  for (crui::Layer* layer_beneath : layers_beneath_)
    layer_beneath->SetTransform(transform);
}

void View::SetPaintToLayer(crui::LayerType layer_type) {
  // Avoid re-creating the layer if unnecessary.
  if (paint_to_layer_explicitly_set_) {
    CR_DCHECK(layer() != nullptr);
    if (layer()->type() == layer_type)
      return;
  }

  DestroyLayerImpl(LayerChangeNotifyBehavior::DONT_NOTIFY);
  paint_to_layer_explicitly_set_ = true;

  if (layer_type != LAYER_NOT_DRAWN) {
    // Gaining a layer means becoming a paint root. We must remove ourselves
    // from our old paint root, if we had one. Traverse up view tree to find old
    // paint root.
    View* old_paint_root = parent_;
    while (old_paint_root && !old_paint_root->IsPaintRoot())
      old_paint_root = old_paint_root->parent_;

    // Remove our and our children's bounds from the old tree. This will also
    // mark all of our bounds as dirty.
    if (old_paint_root && old_paint_root->bounds_tree_)
      RemoveRootBounds(old_paint_root->bounds_tree_.get());
  } else {
    // Losing a layer means we are no longer a paint root, so delete our
    // bounds tree and mark ourselves as dirty for future insertion into our
    // new paint root's bounds tree.
    bounds_tree_.reset();
    SetRootBoundsDirty(true);
  }

  // We directly call |CreateLayer()| here to pass |layer_type|. A call to
  // |CreateOrDestroyLayer()| is therefore not necessary.
  CreateLayer(layer_type);

  // Notify the parent chain about the layer change.
  NotifyParentsOfLayerChange();
}

void View::DestroyLayer() {
  paint_to_layer_explicitly_set_ = false;
  CreateOrDestroyLayer();
}

void View::AddLayerBeneathView(crui::Layer* new_layer) {
  CR_DCHECK(new_layer);
  CR_DCHECK(!cr::Contains(layers_beneath_, new_layer)) 
      << "Layer already added.";

  new_layer->AddObserver(this);
  new_layer->SetVisible(GetVisible());
  layers_beneath_.push_back(new_layer);

  // If painting to a layer already, ensure |new_layer| gets added and stacked
  // correctly. If not, this will happen on layer creation.
  if (layer()) {
    crui::Layer* parent_layer = layer()->parent();
    // Note that |new_layer| may have already been added to the parent, for
    // example when the layer of a LayerOwner is recreated.
    if (parent_layer && parent_layer != new_layer->parent())
      parent_layer->Add(new_layer);
    new_layer->SetBounds(gfx::Rect(new_layer->size()) +
                         layer()->bounds().OffsetFromOrigin());
    if (parent())
      parent()->ReorderLayers();
  }

  CreateOrDestroyLayer();

  layer()->SetFillsBoundsOpaquely(false);
}

void View::RemoveLayerBeneathView(crui::Layer* old_layer) {
  RemoveLayerBeneathViewKeepInLayerTree(old_layer);

  // Note that |old_layer| may have already been removed from its parent.
  crui::Layer* parent_layer = layer()->parent();
  if (parent_layer && parent_layer == old_layer->parent())
    parent_layer->Remove(old_layer);

  CreateOrDestroyLayer();
}

void View::RemoveLayerBeneathViewKeepInLayerTree(crui::Layer* old_layer) {
  auto layer_pos =
      std::find(layers_beneath_.begin(), layers_beneath_.end(), old_layer);
  CR_DCHECK(layer_pos != layers_beneath_.end())
      << "Attempted to remove a layer that was never added.";
  layers_beneath_.erase(layer_pos);
  old_layer->RemoveObserver(this);
}

std::vector<crui::Layer*> View::GetLayersInOrder() {
  // If not painting to a layer, there are no layers immediately related to this
  // view.
  if (!layer())
    return {};

  std::vector<crui::Layer*> result;
  for (crui::Layer* layer_beneath : layers_beneath_)
    result.push_back(layer_beneath);
  result.push_back(layer());

  return result;
}

void View::LayerDestroyed(crui::Layer* layer) {
  // Only layers added with |AddLayerBeneathView()| are observed so |layer| can
  // safely be removed.
  RemoveLayerBeneathView(layer);
}

std::unique_ptr<crui::Layer> View::RecreateLayer() {
  std::unique_ptr<crui::Layer> old_layer = LayerOwner::RecreateLayer();
  Widget* widget = GetWidget();
  if (widget)
    widget->LayerTreeChanged();
  return old_layer;
}

// RTL positioning -------------------------------------------------------------

gfx::Rect View::GetMirroredBounds() const {
  gfx::Rect bounds(bounds_);
  bounds.set_x(GetMirroredX());
  return bounds;
}

gfx::Point View::GetMirroredPosition() const {
  return gfx::Point(GetMirroredX(), y());
}

int View::GetMirroredX() const {
  return parent_ ? parent_->GetMirroredXForRect(bounds_) : x();
}

int View::GetMirroredXForRect(const gfx::Rect& rect) const {
  return crui::i18n::IsRTL() ? (width() - rect.x() - rect.width()) : rect.x();
}

gfx::Rect View::GetMirroredRect(const gfx::Rect& rect) const {
  gfx::Rect mirrored_rect = rect;
  mirrored_rect.set_x(GetMirroredXForRect(rect));
  return mirrored_rect;
}

int View::GetMirroredXInView(int x) const {
  return crui::i18n::IsRTL() ? width() - x : x;
}

int View::GetMirroredXWithWidthInView(int x, int w) const {
  return crui::i18n::IsRTL() ? width() - x - w : x;
}

// Layout ----------------------------------------------------------------------

void View::Layout() {
  needs_layout_ = false;

  // If we have a layout manager, let it handle the layout for us.
  if (layout_manager_)
    layout_manager_->Layout(this);

  // Make sure to propagate the Layout() call to any children that haven't
  // received it yet through the layout manager and need to be laid out. This
  // is needed for the case when the child requires a layout but its bounds
  // weren't changed by the layout manager. If there is no layout manager, we
  // just propagate the Layout() call down the hierarchy, so whoever receives
  // the call can take appropriate action.
  internal::ScopedChildrenLock lock(this);
  for (auto* child : children_) {
    if (child->needs_layout_ || !layout_manager_) {
      ///TRACE_EVENT1("views", "View::Layout", "class", child->GetClassName());
      child->needs_layout_ = false;
      child->Layout();
    }
  }
}

void View::InvalidateLayout() {
  // Always invalidate up. This is needed to handle the case of us already being
  // valid, but not our parent.
  needs_layout_ = true;
  if (layout_manager_)
    layout_manager_->InvalidateLayout();

  if (parent_) {
    parent_->InvalidateLayout();
  } else {
    Widget* widget = GetWidget();
    if (widget)
      widget->ScheduleLayout();
  }
}

LayoutManager* View::GetLayoutManager() const {
  return layout_manager_.get();
}

void View::SetLayoutManager(std::nullptr_t) {
  SetLayoutManagerImpl(nullptr);
}

// Attributes ------------------------------------------------------------------

const View* View::GetViewByID(int id) const {
  if (id == id_)
    return const_cast<View*>(this);

  internal::ScopedChildrenLock lock(this);
  for (auto* child : children_) {
    const View* view = child->GetViewByID(id);
    if (view)
      return view;
  }
  return nullptr;
}

View* View::GetViewByID(int id) {
  return const_cast<View*>(const_cast<const View*>(this)->GetViewByID(id));
}

void View::SetID(int id) {
  if (id == id_)
    return;

  id_ = id;

  OnPropertyChanged(&id_, kPropertyEffectsNone);
}

PropertyChangedSubscription View::AddIDChangedCallback(
    PropertyChangedCallback callback) {
  return AddPropertyChangedCallback(&id_, callback);
}

void View::SetGroup(int gid) {
  // Don't change the group id once it's set.
  CR_DCHECK(group_ == -1 || group_ == gid);
  if (group_ != gid) {
    group_ = gid;
    OnPropertyChanged(&group_, kPropertyEffectsNone);
  }
}

int View::GetGroup() const {
  return group_;
}

PropertyChangedSubscription View::AddGroupChangedCallback(
    PropertyChangedCallback callback) {
  return AddPropertyChangedCallback(&group_, callback);
}

bool View::IsGroupFocusTraversable() const {
  return true;
}

void View::GetViewsInGroup(int group, Views* views) {
  if (group_ == group)
    views->push_back(this);

  internal::ScopedChildrenLock lock(this);
  for (auto* child : children_)
    child->GetViewsInGroup(group, views);
}

View* View::GetSelectedViewForGroup(int group) {
  Views views;
  GetWidget()->GetRootView()->GetViewsInGroup(group, &views);
  return views.empty() ? nullptr : views[0];
}

// Coordinate conversion -------------------------------------------------------

// static
void View::ConvertPointToTarget(const View* source,
                                const View* target,
                                gfx::Point* point) {
  CR_DCHECK(source);
  CR_DCHECK(target);
  if (source == target)
    return;

  const View* root = GetHierarchyRoot(target);
  CR_CHECK(GetHierarchyRoot(source) == root);

  if (source != root)
    source->ConvertPointForAncestor(root, point);

  if (target != root)
    target->ConvertPointFromAncestor(root, point);
}

// static
void View::ConvertRectToTarget(const View* source,
                               const View* target,
                               gfx::RectF* rect) {
  CR_DCHECK(source);
  CR_DCHECK(target);
  if (source == target)
    return;

  const View* root = GetHierarchyRoot(target);
  CR_CHECK(GetHierarchyRoot(source) == root);

  if (source != root)
    source->ConvertRectForAncestor(root, rect);

  if (target != root)
    target->ConvertRectFromAncestor(root, rect);
}

// static
void View::ConvertPointToWidget(const View* src, gfx::Point* p) {
  CR_DCHECK(src);
  CR_DCHECK(p);

  src->ConvertPointForAncestor(nullptr, p);
}

// static
void View::ConvertPointFromWidget(const View* dest, gfx::Point* p) {
  CR_DCHECK(dest);
  CR_DCHECK(p);

  dest->ConvertPointFromAncestor(nullptr, p);
}

// static
void View::ConvertPointToScreen(const View* src, gfx::Point* p) {
  CR_DCHECK(src);
  CR_DCHECK(p);

  // If the view is not connected to a tree, there's nothing we can do.
  const Widget* widget = src->GetWidget();
  if (widget) {
    ConvertPointToWidget(src, p);
    *p += widget->GetClientAreaBoundsInScreen().OffsetFromOrigin();
  }
}

// static
void View::ConvertPointFromScreen(const View* dst, gfx::Point* p) {
  CR_DCHECK(dst);
  CR_DCHECK(p);

  const views::Widget* widget = dst->GetWidget();
  if (!widget)
    return;
  *p -= widget->GetClientAreaBoundsInScreen().OffsetFromOrigin();
  ConvertPointFromWidget(dst, p);
}

// static
void View::ConvertRectToScreen(const View* src, gfx::Rect* rect) {
  CR_DCHECK(src);
  CR_DCHECK(rect);

  gfx::Point new_origin = rect->origin();
  views::View::ConvertPointToScreen(src, &new_origin);
  rect->set_origin(new_origin);
}

gfx::Rect View::ConvertRectToParent(const gfx::Rect& rect) const {
  // This mapping returns the enclosing rect, which is good because pixels that
  // partially occupy in the parent should be included.
  gfx::Rect x_rect = GetTransform().MapRect(rect);
  x_rect.Offset(GetMirroredPosition().OffsetFromOrigin());
  return x_rect;
}

gfx::Rect View::ConvertRectToWidget(const gfx::Rect& rect) const {
  gfx::Rect x_rect = rect;
  for (const View* v = this; v; v = v->parent_)
    x_rect = v->ConvertRectToParent(x_rect);
  return x_rect;
}

// Painting --------------------------------------------------------------------

void View::SchedulePaint() {
  SchedulePaintInRect(GetLocalBounds());
}

void View::SchedulePaintInRect(const gfx::Rect& rect) {
  needs_paint_ = true;
  SchedulePaintInRectImpl(rect);
}

void View::Paint(gfx::Canvas* canvas, const CullSet& cull_set) {
//  if (!ShouldPaint())
//    return;
//
//  const gfx::Rect& parent_bounds =
//      !parent() ? GetMirroredBounds() : parent()->GetMirroredBounds();
//
//  PaintInfo paint_info = PaintInfo::CreateChildPaintInfo(
//      parent_paint_info, GetMirroredBounds(), parent_bounds.size(),
//      GetPaintScaleType(), !!layer(), needs_paint_);
//
//  needs_paint_ = false;
//
//  const ui::PaintContext& context = paint_info.context();
//  bool is_invalidated = true;
//  if (paint_info.context().CanCheckInvalid() ||
//      base::FeatureList::IsEnabled(features::kEnableViewPaintOptimization)) {
//    // For View paint optimization, do not default to repainting every View in
//    // the View hierarchy if the invalidation rect is empty. Repainting does not
//    // depend on the invalidation rect for View paint optimization.
//#if DCHECK_IS_ON()
//    if (!context.is_pixel_canvas()) {
//      gfx::Vector2d offset;
//      context.Visited(this);
//      View* view = this;
//      while (view->parent() && !view->layer()) {
//        DCHECK(view->GetTransform().IsIdentity());
//        offset += view->GetMirroredPosition().OffsetFromOrigin();
//        view = view->parent();
//      }
//      // The offset in the PaintContext should be the offset up to the paint
//      // root, which we compute and verify here.
//      DCHECK_EQ(context.PaintOffset().x(), offset.x());
//      DCHECK_EQ(context.PaintOffset().y(), offset.y());
//      // The above loop will stop when |view| is the paint root, which should be
//      // the root of the current paint walk, as verified by storing the root in
//      // the PaintContext.
//      DCHECK_EQ(context.RootVisited(), view);
//    }
//#endif
//
//    // If the View wasn't invalidated, don't waste time painting it, the output
//    // would be culled.
//    is_invalidated = paint_info.ShouldPaint();
//  }
//
//  TRACE_EVENT1("views", "View::Paint", "class", GetClassName());
//
//  // If the view is backed by a layer, it should paint with itself as the origin
//  // rather than relative to its parent.
//  // TODO(danakj): Rework clip and transform recorder usage here to use
//  // std::optional once we can do so.
//  ui::ClipRecorder clip_recorder(parent_paint_info.context());
//  if (!layer()) {
//    // Set the clip rect to the bounds of this View, or |clip_path_| if it's
//    // been set. Note that the X (or left) position we pass to ClipRect takes
//    // into consideration whether or not the View uses a right-to-left layout so
//    // that we paint the View in its mirrored position if need be.
//    if (clip_path_.isEmpty()) {
//      clip_recorder.ClipRect(gfx::Rect(paint_info.paint_recording_size()) +
//                             paint_info.offset_from_parent());
//    } else {
//      SkPath clip_path_in_parent = clip_path_;
//
//      // Transform |clip_path_| from local space to parent recording space.
//      gfx::Transform to_parent_recording_space;
//
//      to_parent_recording_space.Translate(paint_info.offset_from_parent());
//      to_parent_recording_space.Scale(
//          SkFloatToScalar(paint_info.paint_recording_scale_x()),
//          SkFloatToScalar(paint_info.paint_recording_scale_y()));
//
//      clip_path_in_parent.transform(
//          SkMatrix(to_parent_recording_space.matrix()));
//      clip_recorder.ClipPathWithAntiAliasing(clip_path_in_parent);
//    }
//  }
//
//  ui::TransformRecorder transform_recorder(context);
//  SetUpTransformRecorderForPainting(paint_info.offset_from_parent(),
//                                    &transform_recorder);
//
//  // Note that the cache is not aware of the offset of the view
//  // relative to the parent since painting is always done relative to
//  // the top left of the individual view.
//  if (is_invalidated ||
//      !paint_cache_.UseCache(context, paint_info.paint_recording_size())) {
//    ui::PaintRecorder recorder(context, paint_info.paint_recording_size(),
//                               paint_info.paint_recording_scale_x(),
//                               paint_info.paint_recording_scale_y(),
//                               &paint_cache_);
//    gfx::Canvas* canvas = recorder.canvas();
//    gfx::ScopedCanvas scoped_canvas(canvas);
//    if (flip_canvas_on_paint_for_rtl_ui_)
//      scoped_canvas.FlipIfRTL(width());
//
//    // Delegate painting the contents of the View to the virtual OnPaint method.
//    OnPaint(canvas);
//  }
//
//  // View::Paint() recursion over the subtree.
//  PaintChildren(paint_info);
  CR_DCHECK(canvas);
  if (!cull_set.ShouldPaint(this))
    return;

  gfx::ScopedCanvas scoped_canvas(canvas);
  // Paint this View and its children, setting the clip rect to the bounds
  // of this View and translating the origin to the local bounds' top left
  // point.
  //
  // Note that the X (or left) position we pass to ClipRectInt takes into
  // consideration whether or not the view uses a right-to-left layout so that
  // we paint our view in its mirrored position if need be.
  gfx::Rect clip_rect = bounds();
  ///clip_rect.Inset(clip_insets_);
  if (parent_)
    clip_rect.set_x(parent_->GetMirroredXForRect(clip_rect));
  canvas->ClipRect(clip_rect);
  if (canvas->IsClipEmpty())
    return;

  // Non-empty clip, translate the graphics such that 0,0 corresponds to where
  // this view is located (related to its parent).
  canvas->Translate(GetMirroredPosition().OffsetFromOrigin());
  canvas->Transform(GetTransform());
  
  // If we are a paint root, we need to construct our own CullSet object for
  // propagation to our children.
  if (IsPaintRoot()) {
    if (!bounds_tree_)
      bounds_tree_.reset(new BoundsTree(2, 5));

    // Recompute our bounds tree as needed.
    UpdateRootBounds(bounds_tree_.get(), gfx::Vector2d());

    // Grab the clip rect from the supplied canvas to use as the query rect.
    gfx::Rect canvas_bounds;
    if (!canvas->GetClipBounds(&canvas_bounds)) {
      CR_NOTREACHED() << "Failed to get clip bounds from the canvas!";
      return;
    }

    // Now query our bounds_tree_ for a set of damaged views that intersect
    // our canvas bounds.
    std::unique_ptr<std::unordered_set<intptr_t> > damaged_views(
        new std::unordered_set<intptr_t>());
    bounds_tree_->AppendIntersectingRecords(
        canvas_bounds, damaged_views.get());
    // Construct a CullSet to wrap the damaged views set, it will delete it
    // for us on scope exit.
    CullSet paint_root_cull_set(std::move(damaged_views));
    // Paint all descendents using our new cull set.
    PaintCommon(canvas, paint_root_cull_set);
  } else {
    // Not a paint root, so we can proceed as normal.
    PaintCommon(canvas, cull_set);
  }
}

void View::PaintCommon(gfx::Canvas* canvas, const CullSet& cull_set)  {
  if (!ShouldPaint())
    return;

  {
    // If the View we are about to paint requested the canvas to be flipped, we
    // should change the transform appropriately.
    // The canvas mirroring is undone once the View is done painting so that we
    // don't pass the canvas with the mirrored transform to Views that didn't
    // request the canvas to be flipped.
    gfx::ScopedCanvas scoped_canvas(canvas);
    if (flip_canvas_on_paint_for_rtl_ui_) {
      canvas->Translate(gfx::Vector2d(width(), 0));
      canvas->Scale(-1, 1);
    }

    OnPaint(canvas);
  }

  PaintChildren(canvas, cull_set);
}

void View::SetBackground(std::unique_ptr<Background> b) {
  background_ = std::move(b);
  SchedulePaint();
}

void View::SetBorder(std::unique_ptr<Border> b) {
  const gfx::Rect old_contents_bounds = GetContentsBounds();
  border_ = std::move(b);

  // Conceptually, this should be PreferredSizeChanged(), but for some view
  // hierarchies that triggers synchronous add/remove operations that are unsafe
  // in some contexts where SetBorder is called.
  //
  // InvalidateLayout() still triggers a re-layout of the view, which should
  // include re-querying its preferred size so in practice this is both safe and
  // has the intended effect.
  if (old_contents_bounds != GetContentsBounds())
    InvalidateLayout();

  SchedulePaint();
}

///const ui::ThemeProvider* View::GetThemeProvider() const {
///  const Widget* widget = GetWidget();
///  return widget ? widget->GetThemeProvider() : nullptr;
///}

///const ui::NativeTheme* View::GetNativeTheme() const {
///  if (native_theme_)
///    return native_theme_;
///
///  if (parent())
///    return parent()->GetNativeTheme();
///
///  const Widget* widget = GetWidget();
///  if (widget)
///    return widget->GetNativeTheme();
///
///  return ui::NativeTheme::GetInstanceForNativeUi();
///}

///void View::SetNativeThemeForTesting(ui::NativeTheme* theme) {
///  ui::NativeTheme* original_native_theme = GetNativeTheme();
///  native_theme_ = theme;
///  if (native_theme_ != original_native_theme)
///    PropagateThemeChanged();
///}

// RTL painting ----------------------------------------------------------------

void View::EnableCanvasFlippingForRTLUI(bool enable) {
  flip_canvas_on_paint_for_rtl_ui_ = enable;
}

// Input -----------------------------------------------------------------------

View* View::GetEventHandlerForPoint(const gfx::Point& point) {
  return GetEventHandlerForRect(gfx::Rect(point, gfx::Size(1, 1)));
}

View* View::GetEventHandlerForRect(const gfx::Rect& rect) {
  return GetEffectiveViewTargeter()->TargetForRect(this, rect);
}

bool View::CanProcessEventsWithinSubtree() const {
  return can_process_events_within_subtree_;
}

View* View::GetTooltipHandlerForPoint(const gfx::Point& point) {
  // TODO(tdanderson): Move this implementation into ViewTargetDelegate.
  if (!HitTestPoint(point) || !CanProcessEventsWithinSubtree())
    return nullptr;

  // Walk the child Views recursively looking for the View that most
  // tightly encloses the specified point.
  View::Views children = GetChildrenInZOrder();
  CR_DCHECK(children_.size() == children.size());
  for (auto* child : cr::Reversed(children)) {
    if (!child->GetVisible())
      continue;

    gfx::Point point_in_child_coords(point);
    ConvertPointToTarget(this, child, &point_in_child_coords);
    View* handler = child->GetTooltipHandlerForPoint(point_in_child_coords);
    if (handler)
      return handler;
  }
  return this;
}

gfx::NativeCursor View::GetCursor(const crui::MouseEvent& event) {
#if defined(MINI_CHROMIUM_OS_WIN)
  static crui::Cursor arrow;
  if (!arrow.platform())
    arrow.SetPlatformCursor(::LoadCursorW(nullptr, IDC_ARROW));
  return arrow;
#else
  return gfx::kNullCursor;
#endif
}

bool View::HitTestPoint(const gfx::Point& point) const {
  return HitTestRect(gfx::Rect(point, gfx::Size(1, 1)));
}

bool View::HitTestRect(const gfx::Rect& rect) const {
  return GetEffectiveViewTargeter()->DoesIntersectRect(this, rect);
}

bool View::IsMouseHovered() const {
  // If we haven't yet been placed in an onscreen view hierarchy, we can't be
  // hovered.
  if (!GetWidget())
    return false;

  // If mouse events are disabled, then the mouse cursor is invisible and
  // is therefore not hovering over this button.
  if (!GetWidget()->IsMouseEventsEnabled())
    return false;

  gfx::Point cursor_pos(display::Screen::GetScreen()->GetCursorScreenPoint());
  ConvertPointFromScreen(this, &cursor_pos);
  return HitTestPoint(cursor_pos);
}

bool View::OnMousePressed(const crui::MouseEvent& event) {
  return false;
}

bool View::OnMouseDragged(const crui::MouseEvent& event) {
  return false;
}

void View::OnMouseReleased(const crui::MouseEvent& event) {
}

void View::OnMouseCaptureLost() {
}

void View::OnMouseMoved(const crui::MouseEvent& event) {
}

void View::OnMouseEntered(const crui::MouseEvent& event) {
}

void View::OnMouseExited(const crui::MouseEvent& event) {
}

void View::SetMouseHandler(View* new_mouse_handler) {
  // |new_mouse_handler| may be nullptr.
  if (parent_)
    parent_->SetMouseHandler(new_mouse_handler);
}

bool View::OnKeyPressed(const crui::KeyEvent& event) {
  return false;
}

bool View::OnKeyReleased(const crui::KeyEvent& event) {
  return false;
}

bool View::OnMouseWheel(const crui::MouseWheelEvent& event) {
  return false;
}

void View::OnKeyEvent(crui::KeyEvent* event) {
  bool consumed = (event->type() == crui::ET_KEY_PRESSED) ? 
      OnKeyPressed(*event) : OnKeyReleased(*event);
  if (consumed)
    event->StopPropagation();
}

void View::OnMouseEvent(crui::MouseEvent* event) {
  switch (event->type()) {
    case crui::ET_MOUSE_PRESSED:
      if (ProcessMousePressed(*event))
        event->SetHandled();
      return;

    case crui::ET_MOUSE_MOVED:
      if ((event->flags() & (crui::EF_LEFT_MOUSE_BUTTON |
                             crui::EF_RIGHT_MOUSE_BUTTON |
                             crui::EF_MIDDLE_MOUSE_BUTTON)) == 0) {
        OnMouseMoved(*event);
        return;
      }
      CR_FALLTHROUGH;
    case crui::ET_MOUSE_DRAGGED:
      ProcessMouseDragged(event);
      return;

    case crui::ET_MOUSE_RELEASED:
      ProcessMouseReleased(*event);
      return;

    case crui::ET_MOUSEWHEEL:
      if (OnMouseWheel(*event->AsMouseWheelEvent()))
        event->SetHandled();
      break;

    case crui::ET_MOUSE_ENTERED:
      ///if (event->flags() & crui::EF_TOUCH_ACCESSIBILITY)
      ///  NotifyAccessibilityEvent(ax::mojom::Event::kHover, true);
      OnMouseEntered(*event);
      break;

    case crui::ET_MOUSE_EXITED:
      OnMouseExited(*event);
      break;

    default:
      return;
  }
}

void View::OnScrollEvent(crui::ScrollEvent* event) {
}

void View::OnTouchEvent(crui::TouchEvent* event) {
  CR_NOTREACHED() << "Views should not receive touch events.";
}

void View::OnGestureEvent(crui::GestureEvent* event) {
}

///const ui::InputMethod* View::GetInputMethod() const {
///  Widget* widget = const_cast<Widget*>(GetWidget());
///  return widget ? const_cast<const ui::InputMethod*>(widget->GetInputMethod())
///                : nullptr;
///}

std::unique_ptr<ViewTargeter> View::SetEventTargeter(
    std::unique_ptr<ViewTargeter> targeter) {
  std::unique_ptr<ViewTargeter> old_targeter = std::move(targeter_);
  targeter_ = std::move(targeter);
  return old_targeter;
}

ViewTargeter* View::GetEffectiveViewTargeter() const {
  CR_DCHECK(GetWidget());
  ViewTargeter* view_targeter = targeter();
  if (!view_targeter)
    view_targeter = GetWidget()->GetRootView()->targeter();
  CR_CHECK(view_targeter);
  return view_targeter;
}

WordLookupClient* View::GetWordLookupClient() {
  return nullptr;
}

bool View::CanAcceptEvent(const crui::Event& event) {
  return IsDrawn();
}

crui::EventTarget* View::GetParentTarget() {
  return parent_;
}

std::unique_ptr<crui::EventTargetIterator> View::GetChildIterator() const {
  return std::make_unique<crui::EventTargetIteratorPtrImpl<View>>(children_);
}

crui::EventTargeter* View::GetEventTargeter() {
  return targeter_.get();
}

void View::ConvertEventToTarget(const crui::EventTarget* target,
                                crui::LocatedEvent* event) const {
  event->ConvertLocationToTarget(this, static_cast<const View*>(target));
}

gfx::PointF View::GetScreenLocationF(const crui::LocatedEvent& event) const {
  CR_DCHECK(this == event.target());
  gfx::Point screen_location(event.location());
  ConvertPointToScreen(this, &screen_location);
  return gfx::PointF(screen_location);
}

// Accelerators ----------------------------------------------------------------

void View::AddAccelerator(const crui::Accelerator& accelerator) {
  if (!accelerators_)
    accelerators_ = std::make_unique<std::vector<crui::Accelerator>>();

  if (!cr::Contains(*accelerators_, accelerator))
    accelerators_->push_back(accelerator);

  RegisterPendingAccelerators();
}

void View::RemoveAccelerator(const crui::Accelerator& accelerator) {
  if (!accelerators_) {
    CR_NOTREACHED() << "Removing non-existing accelerator";
    return;
  }

  auto i(std::find(accelerators_->begin(), accelerators_->end(), accelerator));
  if (i == accelerators_->end()) {
    CR_NOTREACHED() << "Removing non-existing accelerator";
    return;
  }

  size_t index = i - accelerators_->begin();
  accelerators_->erase(i);
  if (index >= registered_accelerator_count_) {
    // The accelerator is not registered to FocusManager.
    return;
  }
  --registered_accelerator_count_;

  // Providing we are attached to a Widget and registered with a focus manager,
  // we should de-register from that focus manager now.
  if (GetWidget() && accelerator_focus_manager_)
    accelerator_focus_manager_->UnregisterAccelerator(accelerator, this);
}

void View::ResetAccelerators() {
  if (accelerators_)
    UnregisterAccelerators(false);
}

bool View::AcceleratorPressed(const crui::Accelerator& accelerator) {
  return false;
}

bool View::CanHandleAccelerators() const {
  const Widget* widget = GetWidget();
  if (!GetEnabled() || !IsDrawn() || !widget || !widget->IsVisible())
    return false;
#if defined(MINI_CHROMIUM_ENABLE_DESKTOP_AURA)
  // Non-ChromeOS aura windows have an associated FocusManagerEventHandler which
  // adds currently focused view as an event PreTarget (see
  // DesktopNativeWidgetAura::InitNativeWidget). However, the focused view isn't
  // always the right view to handle accelerators: It should only handle them
  // when active. Only top level widgets can be active, so for child widgets
  // check if they are focused instead. ChromeOS also behaves different than
  // Linux when an extension popup is about to handle the accelerator.
  bool child = widget && widget->GetTopLevelWidget() != widget;
  bool focus_in_child =
      widget &&
      widget->GetRootView()->Contains(GetFocusManager()->GetFocusedView());
  if ((child && !focus_in_child) || (!child && !widget->IsActive()))
    return false;
#endif
  return true;
}

// Focus -----------------------------------------------------------------------

bool View::HasFocus() const {
  const FocusManager* focus_manager = GetFocusManager();
  return focus_manager && (focus_manager->GetFocusedView() == this);
}

View* View::GetNextFocusableView() {
  return next_focusable_view_;
}

const View* View::GetNextFocusableView() const {
  return next_focusable_view_;
}

View* View::GetPreviousFocusableView() {
  return previous_focusable_view_;
}

void View::SetNextFocusableView(View* view) {
  if (view)
    view->previous_focusable_view_ = this;
  next_focusable_view_ = view;
}

View::FocusBehavior View::GetFocusBehavior() const {
  return focus_behavior_;
}

void View::SetFocusBehavior(FocusBehavior focus_behavior) {
  if (focus_behavior_ == focus_behavior)
    return;

  focus_behavior_ = focus_behavior;
  AdvanceFocusIfNecessary();

  OnPropertyChanged(&focus_behavior_, kPropertyEffectsNone);
}

bool View::IsFocusable() const {
  return focus_behavior_ == FocusBehavior::ALWAYS && GetEnabled() && IsDrawn();
}

bool View::IsAccessibilityFocusable() const {
  return focus_behavior_ != FocusBehavior::NEVER && GetEnabled() && IsDrawn();
}

FocusManager* View::GetFocusManager() {
  Widget* widget = GetWidget();
  return widget ? widget->GetFocusManager() : nullptr;
}

const FocusManager* View::GetFocusManager() const {
  const Widget* widget = GetWidget();
  return widget ? widget->GetFocusManager() : nullptr;
}

void View::RequestFocus() {
  FocusManager* focus_manager = GetFocusManager();
  if (focus_manager) {
    bool focusable = focus_manager->keyboard_accessible()
                         ? IsAccessibilityFocusable()
                         : IsFocusable();
    if (focusable)
      focus_manager->SetFocusedView(this);
  }
}

bool View::SkipDefaultKeyEventProcessing(const crui::KeyEvent& event) {
  return false;
}

FocusTraversable* View::GetFocusTraversable() {
  return nullptr;
}

FocusTraversable* View::GetPaneFocusTraversable() {
  return nullptr;
}

// Tooltips --------------------------------------------------------------------

cr::string16 View::GetTooltipText(const gfx::Point& p) const {
  return cr::string16();
}

// Context menus ---------------------------------------------------------------

void View::ShowContextMenu(const gfx::Point& p,
                           crui::MenuSourceType source_type) {
  ///if (!context_menu_controller_)
  ///  return;
  ///
  ///context_menu_controller_->ShowContextMenuForView(this, p, source_type);
}

// static
bool View::ShouldShowContextMenuOnMousePress() {
  return kContextMenuOnMousePress;
}

gfx::Point View::GetKeyboardContextMenuLocation() {
  gfx::Rect vis_bounds = GetVisibleBounds();
  gfx::Point screen_point(vis_bounds.x() + vis_bounds.width() / 2,
                          vis_bounds.y() + vis_bounds.height() / 2);
  ConvertPointToScreen(this, &screen_point);
  return screen_point;
}

// Drag and drop ---------------------------------------------------------------

bool View::GetDropFormats(int* formats,
                          std::set<crui::ClipboardFormatType>* format_types) {
  return false;
}

bool View::AreDropTypesRequired() {
  return false;
}

bool View::CanDrop(const OSExchangeData& data) {
  // TODO(sky): when I finish up migration, this should default to true.
  return false;
}

void View::OnDragEntered(const crui::DropTargetEvent& event) {
}

int View::OnDragUpdated(const crui::DropTargetEvent& event) {
  return crui::DragDropTypes::DRAG_NONE;
}

void View::OnDragExited() {
}

int View::OnPerformDrop(const crui::DropTargetEvent& event) {
  return crui::DragDropTypes::DRAG_NONE;
}

void View::OnDragDone() {
}

// static
bool View::ExceededDragThreshold(const gfx::Vector2d& delta) {
  return (abs(delta.x()) > GetHorizontalDragThreshold() ||
          abs(delta.y()) > GetVerticalDragThreshold());
}

// Accessibility----------------------------------------------------------------

///ViewAccessibility& View::GetViewAccessibility() {
///  if (!view_accessibility_)
///    view_accessibility_ = ViewAccessibility::Create(this);
///  return *view_accessibility_;
///}
///
///bool View::HandleAccessibleAction(const ui::AXActionData& action_data) {
///  switch (action_data.action) {
///    case ax::mojom::Action::kBlur:
///      if (HasFocus()) {
///        GetFocusManager()->ClearFocus();
///        return true;
///      }
///      break;
///    case ax::mojom::Action::kDoDefault: {
///      const gfx::Point center = GetLocalBounds().CenterPoint();
///      ui::MouseEvent press(ui::ET_MOUSE_PRESSED, center, center,
///                           ui::EventTimeForNow(), ui::EF_LEFT_MOUSE_BUTTON,
///                           ui::EF_LEFT_MOUSE_BUTTON);
///      OnEvent(&press);
///      ui::MouseEvent release(ui::ET_MOUSE_RELEASED, center, center,
///                             ui::EventTimeForNow(), ui::EF_LEFT_MOUSE_BUTTON,
///                             ui::EF_LEFT_MOUSE_BUTTON);
///      OnEvent(&release);
///      return true;
///    }
///    case ax::mojom::Action::kFocus:
///      if (IsAccessibilityFocusable()) {
///        RequestFocus();
///        return true;
///      }
///      break;
///    case ax::mojom::Action::kScrollToMakeVisible:
///      ScrollRectToVisible(GetLocalBounds());
///      return true;
///    case ax::mojom::Action::kShowContextMenu:
///      ShowContextMenu(GetBoundsInScreen().CenterPoint(),
///                      ui::MENU_SOURCE_KEYBOARD);
///      return true;
///    default:
///      // Some actions are handled by subclasses of View.
///      break;
///  }
///
///  return false;
///}

gfx::NativeViewAccessible View::GetNativeViewAccessible() {
  ///return GetViewAccessibility().GetNativeObject();
  return nullptr;
}

///
///void View::NotifyAccessibilityEvent(ax::mojom::Event event_type,
///                                    bool send_native_event) {
///  AXEventManager::Get()->NotifyViewEvent(this, event_type);
///
///  if (send_native_event && GetWidget())
///    GetViewAccessibility().NotifyAccessibilityEvent(event_type);
///
///  OnAccessibilityEvent(event_type);
///}
///
///void View::OnAccessibilityEvent(ax::mojom::Event event_type) {}

// Scrolling -------------------------------------------------------------------

void View::ScrollRectToVisible(const gfx::Rect& rect) {
  if (parent_)
    parent_->ScrollRectToVisible(rect + bounds().OffsetFromOrigin());
}

void View::ScrollViewToVisible() {
  ScrollRectToVisible(GetLocalBounds());
}

void View::AddObserver(ViewObserver* observer) {
  CR_CHECK(observer);
  observers_.AddObserver(observer);
}

void View::RemoveObserver(ViewObserver* observer) {
  observers_.RemoveObserver(observer);
}

bool View::HasObserver(const ViewObserver* observer) const {
  return observers_.HasObserver(observer);
}

////////////////////////////////////////////////////////////////////////////////
// View, protected:

// Size and disposition --------------------------------------------------------

gfx::Size View::CalculatePreferredSize() const {
  if (layout_manager_)
    return layout_manager_->GetPreferredSize(this);
  return gfx::Size();
}

void View::PreferredSizeChanged() {
  if (parent_)
    parent_->ChildPreferredSizeChanged(this);
  // Since some layout managers (specifically AnimatingLayoutManager) can react
  // to InvalidateLayout() by doing calculations and since the parent can
  // potentially change preferred size, etc. as a result of calling
  // ChildPreferredSizeChanged(), postpone invalidation until the events have
  // run all the way up the hierarchy.
  InvalidateLayout();
  for (ViewObserver& observer : observers_)
    observer.OnViewPreferredSizeChanged(this);
}

bool View::GetNeedsNotificationWhenVisibleBoundsChange() const {
  return false;
}

void View::OnVisibleBoundsChanged() {
}

// Tree operations -------------------------------------------------------------

void View::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) {
}

void View::VisibilityChanged(View* starting_from, bool is_visible) {
}

void View::NativeViewHierarchyChanged() {
  FocusManager* focus_manager = GetFocusManager();
  if (accelerator_focus_manager_ != focus_manager) {
    UnregisterAccelerators(true);

    if (focus_manager)
      RegisterPendingAccelerators();
  }
}

void View::AddedToWidget() {}

void View::RemovedFromWidget() {}

// Painting --------------------------------------------------------------------

void View::OnDidSchedulePaint(const gfx::Rect& rect) {}

void View::PaintChildren(gfx::Canvas* canvas, const CullSet& cull_set) {
  ///TRACE_EVENT1("views", "View::PaintChildren", "class", GetClassName());
  RecursivePaintHelper(&View::Paint, canvas, cull_set);
}

void View::OnPaint(gfx::Canvas* canvas) {
  ///TRACE_EVENT1("views", "View::OnPaint", "class", GetClassName());
  OnPaintBackground(canvas);
  OnPaintBorder(canvas);
}

void View::OnPaintBackground(gfx::Canvas* canvas) {
  if (background_) {
    ///TRACE_EVENT0("views", "View::OnPaintBackground");
    background_->Paint(canvas, this);
  }
}

void View::OnPaintBorder(gfx::Canvas* canvas) {
  if (border_) {
    ///TRACE_EVENT0("views", "View::OnPaintBorder");
    border_->Paint(*this, canvas);
  }
}

bool View::IsPaintRoot() {
  return paint_to_layer_explicitly_set_ || !parent_;
}

// Accelerated Painting --------------------------------------------------------

View::LayerOffsetData View::CalculateOffsetToAncestorWithLayer(
    crui::Layer** layer_parent) {
  if (layer()) {
    if (layer_parent)
      *layer_parent = layer();
    return LayerOffsetData(layer()->device_scale_factor());
  }
  if (!parent_)
    return LayerOffsetData();

  LayerOffsetData offset_data =
      parent_->CalculateOffsetToAncestorWithLayer(layer_parent);

  return offset_data + GetMirroredPosition().OffsetFromOrigin();
}

void View::UpdateParentLayer() {
  if (!layer())
    return;
  
  crui::Layer* parent_layer = nullptr;
  
  if (parent_)
    parent_->CalculateOffsetToAncestorWithLayer(&parent_layer);
  
  ReparentLayer(parent_layer);
}

void View::MoveLayerToParent(crui::Layer* parent_layer,
                             const LayerOffsetData& offset_data) {
  LayerOffsetData local_offset_data(offset_data);
  if (parent_layer != layer())
    local_offset_data += GetMirroredPosition().OffsetFromOrigin();

  if (layer() && parent_layer != layer()) {
    // Adding the main layer can trigger a call to |SnapLayerToPixelBoundary()|.
    // That method assumes layers beneath have already been added. Therefore
    // layers beneath must be added first here. See crbug.com/961212.
    for (crui::Layer* layer_beneath : layers_beneath_)
      parent_layer->Add(layer_beneath);
    parent_layer->Add(layer());

    SetLayerBounds(size(), local_offset_data);
  } else {
    internal::ScopedChildrenLock lock(this);
    for (auto* child : GetChildrenInZOrder())
      child->MoveLayerToParent(parent_layer, local_offset_data);
  }
}

void View::UpdateLayerVisibility() {
  bool visible = visible_;
  for (const View* v = parent_; visible && v && !v->layer(); v = v->parent_)
    visible = v->GetVisible();

  UpdateChildLayerVisibility(visible);
}

void View::UpdateChildLayerVisibility(bool ancestor_visible) {
  const bool layers_visible = ancestor_visible && visible_;
  if (layer()) {
    layer()->SetVisible(layers_visible);
    for (crui::Layer* layer_beneath : layers_beneath_)
      layer_beneath->SetVisible(layers_visible);
  } else {
    internal::ScopedChildrenLock lock(this);
    for (auto* child : children_)
      child->UpdateChildLayerVisibility(layers_visible);
  }
}

void View::DestroyLayerImpl(LayerChangeNotifyBehavior notify_parents) {
  // Normally, adding layers beneath will trigger painting to a layer. It would
  // leave this view in an inconsistent state if its layer were destroyed while
  // layers beneath were still present. So, assume this doesn't happen.
  CR_DCHECK(layers_beneath_.empty());
  
  if (!layer())
    return;
  
  // Copy children(), since the loop below will mutate its result.
  std::vector<crui::Layer*> children = layer()->children();
  crui::Layer* new_parent = layer()->parent();
  for (auto* child : children) {
    layer()->Remove(child);
    if (new_parent)
      new_parent->Add(child);
  }
  
  LayerOwner::DestroyLayer();
  
  if (new_parent)
    ReorderLayers();
  
  UpdateChildLayerBounds(CalculateOffsetToAncestorWithLayer(nullptr));
  
  SchedulePaint();
  
  // Notify the parent chain about the layer change.
  if (notify_parents == LayerChangeNotifyBehavior::NOTIFY)
    NotifyParentsOfLayerChange();
  
  Widget* widget = GetWidget();
  if (widget)
    widget->LayerTreeChanged();
}

void View::NotifyParentsOfLayerChange() {
  // Notify the parent chain about the layer change.
  View* view_parent = parent();
  while (view_parent) {
    view_parent->OnChildLayerChanged(this);
    view_parent = view_parent->parent();
  }
}

void View::UpdateChildLayerBounds(const LayerOffsetData& offset_data) {
  if (layer()) {
    SetLayerBounds(size(), offset_data);
  } else {
    internal::ScopedChildrenLock lock(this);
    for (auto* child : children_) {
      child->UpdateChildLayerBounds(
          offset_data + child->GetMirroredPosition().OffsetFromOrigin());
    }
  }
}

void View::OnPaintLayer(gfx::Canvas* canvas) {
  ///PaintFromPaintRoot(canvas);
  if (!layer() || !layer()->fills_bounds_opaquely())
    canvas->DrawColor(SK_ColorBLACK, SkXfermode::kClear_Mode);
  PaintCommon(canvas, CullSet());
}

void View::OnDeviceScaleFactorChanged(float old_device_scale_factor,
                                      float new_device_scale_factor) {
  snap_layer_to_pixel_boundary_ =
      (new_device_scale_factor - std::floor(new_device_scale_factor)) != 0.0f;

  if (!layer())
    return;
  
  // There can be no subpixel offset if the layer has no parent.
  if (!parent() || !layer()->parent())
    return;
  
  ///if (layer()->parent() && layer()->GetCompositor() &&
  ///    layer()->GetCompositor()->is_pixel_canvas()) {
  ///  LayerOffsetData offset_data(
  ///      parent()->CalculateOffsetToAncestorWithLayer(nullptr));
  ///  offset_data += GetMirroredPosition().OffsetFromOrigin();
  ///  SnapLayerToPixelBoundary(offset_data);
  ///} else {
    SnapLayerToPixelBoundary(LayerOffsetData());
  ///}
}

void View::CreateOrDestroyLayer() {
  if (paint_to_layer_explicitly_set_ || paint_to_layer_for_transform_ ||
      !layers_beneath_.empty()) {
    // If we need to paint to a layer, make sure we have one.
    if (!layer())
      CreateLayer(crui::LAYER_TEXTURED);
  } else if (layer()) {
    // If we don't, make sure we delete our layer.
    DestroyLayerImpl(LayerChangeNotifyBehavior::NOTIFY);
  }
}

void View::ReorderLayers() {
  View* v = this;
  while (v && !v->layer())
    v = v->parent();

  Widget* widget = GetWidget();
  if (!v) {
    if (widget) {
      crui::Layer* layer = widget->GetLayer();
      if (layer)
        widget->GetRootView()->ReorderChildLayers(layer);
    }
  } else {
    v->ReorderChildLayers(v->layer());
  }

  if (widget) {
    // Reorder the widget's child NativeViews in case a child NativeView is
    // associated with a view (e.g. via a NativeViewHost). Always do the
    // reordering because the associated NativeView's layer (if it has one)
    // is parented to the widget's layer regardless of whether the host view has
    // an ancestor with a layer.
    widget->ReorderNativeViews();
  }
}

void View::ReorderChildLayers(crui::Layer* parent_layer) {
  if (layer() && layer() != parent_layer) {
    CR_DCHECK(parent_layer == layer()->parent());
    parent_layer->StackAtBottom(layer());
    for (crui::Layer* layer_beneath : layers_beneath_)
      parent_layer->StackAtBottom(layer_beneath);
  } else {
    // Iterate backwards through the children so that a child with a layer
    // which is further to the back is stacked above one which is further to
    // the front.
    View::Views children = GetChildrenInZOrder();
    CR_DCHECK(children_.size() == children.size());
    for (auto* child : cr::Reversed(children))
      child->ReorderChildLayers(parent_layer);
  }
}

void View::OnChildLayerChanged(View* child) {}

// Input -----------------------------------------------------------------------

View::DragInfo* View::GetDragInfo() {
  return parent_ ? parent_->GetDragInfo() : nullptr;
}

// Focus -----------------------------------------------------------------------

void View::OnFocus() {
  // TODO(beng): Investigate whether it's possible for us to move this to
  //             Focus().
  // By default, we clear the native focus. This ensures that no visible native
  // view as the focus and that we still receive keyboard inputs.
  FocusManager* focus_manager = GetFocusManager();
  if (focus_manager)
    focus_manager->ClearNativeFocus();

  // TODO(beng): Investigate whether it's possible for us to move this to
  //             Focus().
  // Notify assistive technologies of the focus change.
  ///NotifyAccessibilityEvent(ax::mojom::Event::kFocus, true);
}

void View::OnBlur() {
}

void View::Focus() {
  OnFocus();
  ScrollViewToVisible();

  for (ViewObserver& observer : observers_)
    observer.OnViewFocused(this);
}

void View::Blur() {
  ViewTracker tracker(this);
  OnBlur();

  if (tracker.view()) {
    for (ViewObserver& observer : observers_)
      observer.OnViewBlurred(this);
  }
}

// Tooltips --------------------------------------------------------------------

void View::TooltipTextChanged() {
  Widget* widget = GetWidget();
  // TooltipManager may be null if there is a problem creating it.
  if (widget && widget->GetTooltipManager())
    widget->GetTooltipManager()->TooltipTextChanged(this);
}

// Drag and drop ---------------------------------------------------------------

int View::GetDragOperations(const gfx::Point& press_pt) {
  return drag_controller_ ?
      drag_controller_->GetDragOperationsForView(this, press_pt) :
      crui::DragDropTypes::DRAG_NONE;
}

void View::WriteDragData(const gfx::Point& press_pt, OSExchangeData* data) {
  CR_DCHECK(drag_controller_);
  drag_controller_->WriteDragDataForView(this, press_pt, data);
}

bool View::InDrag() const {
  const Widget* widget = GetWidget();
  return widget ? widget->dragged_view() == this : false;
}

int View::GetHorizontalDragThreshold() {
  // TODO(jennyz): This value may need to be adjusted for different platforms
  // and for different display density.
  return kDefaultHorizontalDragThreshold;
}

int View::GetVerticalDragThreshold() {
  // TODO(jennyz): This value may need to be adjusted for different platforms
  // and for different display density.
  return kDefaultVerticalDragThreshold;
}

///PaintInfo::ScaleType View::GetPaintScaleType() const {
///  return PaintInfo::ScaleType::kScaleWithEdgeSnapping;
///}

void View::HandlePropertyChangeEffects(PropertyEffects effects) {
  if (effects & kPropertyEffectsPreferredSizeChanged)
    PreferredSizeChanged();
  if (effects & kPropertyEffectsLayout)
    InvalidateLayout();
  if (effects & kPropertyEffectsPaint)
    SchedulePaint();
  OnHandlePropertyChangeEffects(effects);
}

PropertyChangedSubscription View::AddPropertyChangedCallback(
    PropertyKey property,
    PropertyChangedCallback callback) {
  auto entry = property_changed_vectors_.find(property);
  if (entry == property_changed_vectors_.end()) {
    entry = property_changed_vectors_
                .emplace(property, std::make_unique<PropertyChangedCallbacks>())
                .first;
  }
  PropertyChangedCallbacks* property_changed_callbacks = entry->second.get();

  return property_changed_callbacks->Add(std::move(callback));
}

void View::OnPropertyChanged(PropertyKey property,
                             PropertyEffects property_effects) {
  if (property_effects != kPropertyEffectsNone)
    HandlePropertyChangeEffects(property_effects);

  auto entry = property_changed_vectors_.find(property);
  if (entry == property_changed_vectors_.end())
    return;

  PropertyChangedCallbacks* property_changed_callbacks = entry->second.get();
  property_changed_callbacks->Notify();
}

////////////////////////////////////////////////////////////////////////////////
// View, private:

// DropInfo --------------------------------------------------------------------

void View::DragInfo::Reset() {
  possible_drag = false;
  start_pt = gfx::Point();
}

void View::DragInfo::PossibleDrag(const gfx::Point& p) {
  possible_drag = true;
  start_pt = p;
}

// Painting --------------------------------------------------------------------

void View::SchedulePaintInRectImpl(const gfx::Rect& rect) {
  OnDidSchedulePaint(rect);
  if (!visible_)
    return;
  if (layer()) {
    ///layer()->SchedulePaint(rect);
  } else if (parent_) {
    // Translate the requested paint rect to the parent's coordinate system
    // then pass this notification up to the parent.
    parent_->SchedulePaintInRectImpl(ConvertRectToParent(rect));
  }
}

void View::SchedulePaintBoundsChanged(bool size_changed) {
  if (!visible_)
    return;

  // If we have a layer and the View's size did not change, we do not need to
  // schedule any paints since the layer will be redrawn at its new location
  // during the next Draw() cycle in the compositor.
  if (!layer() || size_changed) {
    // Otherwise, if the size changes or we don't have a layer then we need to
    // use SchedulePaint to invalidate the area occupied by the View.
    SchedulePaint();
  } else {
    // The compositor doesn't Draw() until something on screen changes, so
    // if our position changes but nothing is being animated on screen, then
    // tell the compositor to redraw the scene. We know layer() exists due to
    // the above if clause.
    ///layer()->ScheduleDraw();
  }
}

void View::SchedulePaintOnParent() {
  if (parent_) {
    // Translate the requested paint rect to the parent's coordinate system
    // then pass this notification up to the parent.
    parent_->SchedulePaintInRect(ConvertRectToParent(GetLocalBounds()));
  }
}

bool View::ShouldPaint() const {
  return visible_ && !size().IsEmpty();
}

///void View::SetUpTransformRecorderForPainting(
///    const gfx::Vector2d& offset_from_parent,
///    crui::TransformRecorder* recorder) const {
///  // If the view is backed by a layer, it should paint with itself as the origin
///  // rather than relative to its parent.
///  if (layer())
///    return;
///
///  // Translate the graphics such that 0,0 corresponds to where this View is
///  // located relative to its parent.
///  gfx::Transform transform_from_parent;
///  transform_from_parent.Translate(offset_from_parent.x(),
///                                  offset_from_parent.y());
///  recorder->Transform(transform_from_parent);
///}

void View::RecursivePaintHelper(void (View::*func)(gfx::Canvas*, const CullSet&),
                                gfx::Canvas* canvas,
                                const CullSet& cull_set) {
  View::Views children = GetChildrenInZOrder();
  CR_DCHECK(children_.size() == children.size());
  for (auto* child : children) {
    if (!child->layer())
      (child->*func)(canvas, cull_set);
  }
}

void View::PaintFromPaintRoot(gfx::Canvas* canvas) {
  ///PaintInfo paint_info = PaintInfo::CreateRootPaintInfo(
  ///    parent_context, layer() ? layer()->size() : size());
  Paint(canvas, CullSet());
  ///if (base::CommandLine::ForCurrentProcess()->HasSwitch(
  ///        switches::kDrawViewBoundsRects))
  ///  PaintDebugRects(paint_info);
}

///void View::PaintDebugRects(const PaintInfo& parent_paint_info) {
///  if (!ShouldPaint())
///    return;
///
///  const gfx::Rect& parent_bounds = (layer() || !parent())
///                                       ? GetMirroredBounds()
///                                       : parent()->GetMirroredBounds();
///  PaintInfo paint_info = PaintInfo::CreateChildPaintInfo(
///      parent_paint_info, GetMirroredBounds(), parent_bounds.size(),
///      GetPaintScaleType(), !!layer());
///
///  const ui::PaintContext& context = paint_info.context();
///
///  ui::TransformRecorder transform_recorder(context);
///  SetUpTransformRecorderForPainting(paint_info.offset_from_parent(),
///                                    &transform_recorder);
///
///  RecursivePaintHelper(&View::PaintDebugRects, paint_info);
///
///  // Draw outline rects for debugging.
///  ui::PaintRecorder recorder(context, paint_info.paint_recording_size(),
///                             paint_info.paint_recording_scale_x(),
///                             paint_info.paint_recording_scale_y(),
///                             &paint_cache_);
///  gfx::Canvas* canvas = recorder.canvas();
///  const float scale = canvas->UndoDeviceScaleFactor();
///  gfx::RectF outline_rect(ScaleToEnclosedRect(GetLocalBounds(), scale));
///  gfx::RectF content_outline_rect(
///      ScaleToEnclosedRect(GetContentsBounds(), scale));
///  if (content_outline_rect != outline_rect) {
///    content_outline_rect.Inset(0.5f, 0.5f);
///    const SkColor content_color = SkColorSetARGB(0x30, 0, 0, 0xff);
///    canvas->DrawRect(content_outline_rect, content_color);
///  }
///  outline_rect.Inset(0.5f, 0.5f);
///  const SkColor color = SkColorSetARGB(0x30, 0xff, 0, 0);
///  canvas->DrawRect(outline_rect, color);
///}
///
// Tree operations -------------------------------------------------------------

void View::AddChildViewAtImpl(View* view, int index) {
  CR_CHECK(view != this) << "You cannot add a view as its own child";
  CR_DCHECK(index >= 0);
  CR_DCHECK(static_cast<size_t>(index) <= children_.size());

  // TODO(https://crbug.com/942298): Should just DCHECK(!view->parent_);.
  View* parent = view->parent_;
  if (parent == this) {
    ReorderChildView(view, index);
    return;
  }

  // Remove |view| from its parent, if any.
  ///crui::NativeTheme* old_theme = nullptr;
  Widget* old_widget = nullptr;
  if (parent) {
    ///old_theme = view->GetNativeTheme();
    old_widget = view->GetWidget();
    parent->DoRemoveChildView(view, true, true, false, this);
  }

  view->parent_ = this;
#if CR_DCHECK_IS_ON()
  CR_DCHECK(!iterating_);
#endif
  const auto pos = children_.insert(std::next(children_.cbegin(), index), view);

  SetFocusSiblings(view, pos);

  // Ensure the layer tree matches the view tree before calling to any client
  // code. This way if client code further modifies the view tree we are in a
  // sane state.
  const bool did_reparent_any_layers = view->UpdateParentLayers();
  Widget* widget = GetWidget();
  if (did_reparent_any_layers && widget)
    widget->LayerTreeChanged();

  ReorderLayers();

  // Make sure the visibility of the child layers are correct.
  // If any of the parent View is hidden, then the layers of the subtree
  // rooted at |this| should be hidden. Otherwise, all the child layers should
  // inherit the visibility of the owner View.
  view->UpdateLayerVisibility();

  // Need to notify the layout manager because one of the callbacks below might
  // want to know the view's new preferred size, minimum size, etc.
  if (layout_manager_)
    layout_manager_->ViewAdded(this, view);

  ViewHierarchyChangedDetails details(true, this, view, parent);

  for (View* v = this; v; v = v->parent_)
    v->ViewHierarchyChangedImpl(false, details);

  view->PropagateAddNotifications(details, widget && widget != old_widget);

  if (widget) {
    ///const crui::NativeTheme* new_theme = view->GetNativeTheme();
    ///if (new_theme != old_theme)
    ///  view->PropagateThemeChanged();
  }

  UpdateTooltip();

  if (widget) {
    RegisterChildrenForVisibleBoundsNotification(view);

    if (view->GetVisible())
      view->SchedulePaint();
  }

  for (ViewObserver& observer : observers_)
    observer.OnChildViewAdded(this, view);
}

void View::DoRemoveChildView(View* view,
                             bool update_focus_cycle,
                             bool update_tool_tip,
                             bool delete_removed_view,
                             View* new_parent) {
  CR_DCHECK(view);

  const auto i = std::find(children_.cbegin(), children_.cend(), view);
  if (i == children_.cend())
    return;

  std::unique_ptr<View> view_to_be_deleted;
  if (update_focus_cycle) {
    View* next_focusable = view->next_focusable_view_;
    View* prev_focusable = view->previous_focusable_view_;
    if (prev_focusable)
      prev_focusable->next_focusable_view_ = next_focusable;
    if (next_focusable)
      next_focusable->previous_focusable_view_ = prev_focusable;
  }

  Widget* widget = GetWidget();
  bool is_removed_from_widget = false;
  if (widget) {
    UnregisterChildrenForVisibleBoundsNotification(view);
    if (view->GetVisible())
      view->SchedulePaint();

    is_removed_from_widget = !new_parent || new_parent->GetWidget() != widget;
    if (is_removed_from_widget)
      widget->NotifyWillRemoveView(view);
  }

  // Make sure the layers belonging to the subtree rooted at |view| get
  // removed.
  view->OrphanLayers();
  if (widget)
    widget->LayerTreeChanged();

  // Need to notify the layout manager because one of the callbacks below might
  // want to know the view's new preferred size, minimum size, etc.
  if (layout_manager_)
    layout_manager_->ViewRemoved(this, view);

  view->PropagateRemoveNotifications(this, new_parent, is_removed_from_widget);
  view->parent_ = nullptr;

  if (delete_removed_view && !view->owned_by_client_)
    view_to_be_deleted.reset(view);

#if CR_DCHECK_IS_ON()
  CR_DCHECK(!iterating_);
#endif
  children_.erase(i);

  if (update_tool_tip)
    UpdateTooltip();

  for (ViewObserver& observer : observers_)
    observer.OnChildViewRemoved(this, view);
}

void View::PropagateRemoveNotifications(View* old_parent,
                                        View* new_parent,
                                        bool is_removed_from_widget) {
  {
    internal::ScopedChildrenLock lock(this);
    for (auto* child : children_) {
      child->PropagateRemoveNotifications(old_parent, new_parent,
                                          is_removed_from_widget);
    }
  }

  ViewHierarchyChangedDetails details(false, old_parent, this, new_parent);
  for (View* v = this; v; v = v->parent_)
    v->ViewHierarchyChangedImpl(true, details);

  if (is_removed_from_widget) {
    RemovedFromWidget();
    for (ViewObserver& observer : observers_)
      observer.OnViewRemovedFromWidget(this);
  }
}

void View::PropagateAddNotifications(const ViewHierarchyChangedDetails& details,
                                     bool is_added_to_widget) {
  {
    internal::ScopedChildrenLock lock(this);
    for (auto* child : children_)
      child->PropagateAddNotifications(details, is_added_to_widget);
  }
  ViewHierarchyChangedImpl(true, details);
  if (is_added_to_widget) {
    AddedToWidget();
    for (ViewObserver& observer : observers_)
      observer.OnViewAddedToWidget(this);
  }
}

void View::PropagateNativeViewHierarchyChanged() {
  {
    internal::ScopedChildrenLock lock(this);
    for (auto* child : children_)
      child->PropagateNativeViewHierarchyChanged();
  }
  NativeViewHierarchyChanged();
}

void View::ViewHierarchyChangedImpl(
    bool register_accelerators,
    const ViewHierarchyChangedDetails& details) {
  if (register_accelerators) {
    if (details.is_add) {
      // If you get this registration, you are part of a subtree that has been
      // added to the view hierarchy.
      if (GetFocusManager())
        RegisterPendingAccelerators();
    } else {
      if (details.child == this)
        UnregisterAccelerators(true);
    }
  }

  ViewHierarchyChanged(details);
  for (ViewObserver& observer : observers_)
    observer.OnViewHierarchyChanged(this, details);

  details.parent->needs_layout_ = true;
}

// Size and disposition --------------------------------------------------------

void View::PropagateVisibilityNotifications(View* start, bool is_visible) {
  {
    internal::ScopedChildrenLock lock(this);
    for (auto* child : children_)
      child->PropagateVisibilityNotifications(start, is_visible);
  }
  VisibilityChangedImpl(start, is_visible);
}

void View::VisibilityChangedImpl(View* starting_from, bool is_visible) {
  VisibilityChanged(starting_from, is_visible);
  for (ViewObserver& observer : observers_)
    observer.OnViewVisibilityChanged(this, starting_from);
}

void View::SnapLayerToPixelBoundary(const LayerOffsetData& offset_data) {
///  if (!layer())
///    return;
///
///#if CR_DCHECK_IS_ON()
///  // We rely on our layers beneath being parented correctly at this point.
///  for (crui::Layer* layer_beneath : layers_beneath_)
///    CR_DCHECK(layer()->parent() == layer_beneath->parent());
///#endif  // DCHECK_IS_ON()
///
///  if (layer()->GetCompositor() && layer()->GetCompositor()->is_pixel_canvas()) {
///    gfx::Vector2dF offset = snap_layer_to_pixel_boundary_ && layer()->parent()
///                                ? offset_data.GetSubpixelOffset()
///                                : gfx::Vector2dF();
///    layer()->SetSubpixelPositionOffset(offset);
///    for (crui::Layer* layer_beneath : layers_beneath_)
///      layer_beneath->SetSubpixelPositionOffset(offset);
///  }
}

// static
void View::RegisterChildrenForVisibleBoundsNotification(View* view) {
  if (view->GetNeedsNotificationWhenVisibleBoundsChange())
    view->RegisterForVisibleBoundsNotification();
  for (View* child : view->children_)
    RegisterChildrenForVisibleBoundsNotification(child);
}

// static
void View::UnregisterChildrenForVisibleBoundsNotification(View* view) {
  if (view->GetNeedsNotificationWhenVisibleBoundsChange())
    view->UnregisterForVisibleBoundsNotification();
  for (View* child : view->children_)
    UnregisterChildrenForVisibleBoundsNotification(child);
}

void View::RegisterForVisibleBoundsNotification() {
  if (registered_for_visible_bounds_notification_)
    return;

  registered_for_visible_bounds_notification_ = true;
  for (View* ancestor = parent_; ancestor; ancestor = ancestor->parent_)
    ancestor->AddDescendantToNotify(this);
}

void View::UnregisterForVisibleBoundsNotification() {
  if (!registered_for_visible_bounds_notification_)
    return;

  registered_for_visible_bounds_notification_ = false;
  for (View* ancestor = parent_; ancestor; ancestor = ancestor->parent_)
    ancestor->RemoveDescendantToNotify(this);
}

void View::AddDescendantToNotify(View* view) {
  CR_DCHECK(view);
  if (!descendants_to_notify_)
    descendants_to_notify_ = std::make_unique<Views>();
  descendants_to_notify_->push_back(view);
}

void View::RemoveDescendantToNotify(View* view) {
  CR_DCHECK(view && descendants_to_notify_);
  auto i(std::find(descendants_to_notify_->begin(),
                   descendants_to_notify_->end(), view));
  CR_DCHECK(i != descendants_to_notify_->end());
  descendants_to_notify_->erase(i);
  if (descendants_to_notify_->empty())
    descendants_to_notify_.reset();
}

void View::SetLayoutManagerImpl(std::unique_ptr<LayoutManager> layout_manager) {
  // Some code keeps a bare pointer to the layout manager for calling
  // derived-class-specific-functions. It's an easy mistake to create a new
  // unique_ptr and re-set the layout manager with a new unique_ptr, which
  // will cause a crash. Re-setting to nullptr is OK.
  CR_CHECK(!layout_manager || layout_manager_ != layout_manager);

  layout_manager_ = std::move(layout_manager);
  if (layout_manager_)
    layout_manager_->Installed(this);
}

void View::SetLayerBounds(const gfx::Size& size,
                          const LayerOffsetData& offset_data) {
  const gfx::Rect bounds = gfx::Rect(size) + offset_data.offset();
  const bool bounds_changed = (bounds != layer()->GetTargetBounds());
  layer()->SetBounds(bounds);
  for (crui::Layer* layer_beneath : layers_beneath_) {
    layer_beneath->SetBounds(gfx::Rect(layer_beneath->size()) +
                             bounds.OffsetFromOrigin());
  }
  SnapLayerToPixelBoundary(offset_data);
  if (bounds_changed) {
    for (ViewObserver& observer : observers_)
      observer.OnLayerTargetBoundsChanged(this);
  }
}


void View::SetRootBoundsDirty(bool origin_changed) {
  root_bounds_dirty_ = true;

  if (origin_changed) {
    // Inform our children that their root bounds are dirty, as their relative
    // coordinates in paint root space have changed since ours have changed.
    for (Views::const_iterator i(children_.begin()); i != children_.end();
         ++i) {
      if (!(*i)->IsPaintRoot())
        (*i)->SetRootBoundsDirty(origin_changed);
    }
  }
}

void View::UpdateRootBounds(BoundsTree* tree, const gfx::Vector2d& offset) {
  // If we're not visible no need to update BoundsTree. When we are made visible
  // the BoundsTree will be updated appropriately.
  if (!visible_)
    return;

  if (!root_bounds_dirty_ && children_.empty())
    return;

  // No need to recompute bounds if we haven't flagged ours as dirty.
  ///TRACE_EVENT1("views", "View::UpdateRootBounds", "class", GetClassName());

  // Add our own offset to the provided offset, for our own bounds update and
  // for propagation to our children if needed.
  gfx::Vector2d view_offset = offset + GetMirroredBounds().OffsetFromOrigin();

  // If our bounds have changed we must re-insert our new bounds to the tree.
  if (root_bounds_dirty_) {
    root_bounds_dirty_ = false;
    gfx::Rect bounds(
        view_offset.x(), view_offset.y(), bounds_.width(), bounds_.height());
    tree->Insert(bounds, reinterpret_cast<intptr_t>(this));
  }

  // Update our children's bounds if needed.
  for (Views::const_iterator i(children_.begin()); i != children_.end(); ++i) {
    // We don't descend in to layer views for bounds recomputation, as they
    // manage their own RTree as paint roots.
    if (!(*i)->IsPaintRoot())
      (*i)->UpdateRootBounds(tree, view_offset);
  }
}

void View::RemoveRootBounds(BoundsTree* tree) {
  tree->Remove(reinterpret_cast<intptr_t>(this));

  root_bounds_dirty_ = true;

  for (Views::const_iterator i(children_.begin()); i != children_.end(); ++i) {
    if (!(*i)->IsPaintRoot())
      (*i)->RemoveRootBounds(tree);
  }
}

View::BoundsTree* View::GetBoundsTreeFromPaintRoot() {
  BoundsTree* bounds_tree = bounds_tree_.get();
  View* paint_root = this;
  while (!bounds_tree && !paint_root->IsPaintRoot()) {
    // Assumption is that if IsPaintRoot() is false then parent_ is valid.
    CR_DCHECK(paint_root);
    paint_root = paint_root->parent_;
    bounds_tree = paint_root->bounds_tree_.get();
  }

  return bounds_tree;
}

// Transformations -------------------------------------------------------------

bool View::GetTransformRelativeTo(const View* ancestor,
                                  gfx::Transform* transform) const {
  const View* p = this;

  while (p && p != ancestor) {
    transform->PostConcat(p->GetTransform());
    gfx::Transform translation;
    translation.Translate(static_cast<float>(p->GetMirroredX()),
                          static_cast<float>(p->y()));
    transform->PostConcat(translation);

    p = p->parent_;
  }

  return p == ancestor;
}

// Coordinate conversion -------------------------------------------------------

bool View::ConvertPointForAncestor(const View* ancestor,
                                   gfx::Point* point) const {
  gfx::Transform trans;
  // TODO(sad): Have some way of caching the transformation results.
  bool result = GetTransformRelativeTo(ancestor, &trans);
  *point = gfx::ToFlooredPoint(trans.MapPoint(gfx::PointF(*point)));
  return result;
}

bool View::ConvertPointFromAncestor(const View* ancestor,
                                    gfx::Point* point) const {
  gfx::Transform trans;
  bool result = GetTransformRelativeTo(ancestor, &trans);
  if (const cr::Optional<gfx::PointF> transformed_point =
    trans.InverseMapPoint(gfx::PointF(*point))) {
    *point = gfx::ToFlooredPoint(transformed_point.value());
  }
  return result;
}

bool View::ConvertRectForAncestor(const View* ancestor,
                                  gfx::RectF* rect) const {
  gfx::Transform trans;
  // TODO(sad): Have some way of caching the transformation results.
  bool result = GetTransformRelativeTo(ancestor, &trans);
  *rect = trans.MapRect(*rect);
  return result;
}

bool View::ConvertRectFromAncestor(const View* ancestor,
                                   gfx::RectF* rect) const {
  gfx::Transform trans;
  bool result = GetTransformRelativeTo(ancestor, &trans);
  *rect = trans.InverseMapRect(*rect).value_or(*rect);
  return result;
}

// Accelerated painting --------------------------------------------------------

void View::CreateLayer(crui::LayerType layer_type) {
  // A new layer is being created for the view. So all the layers of the
  // sub-tree can inherit the visibility of the corresponding view.
  {
    internal::ScopedChildrenLock lock(this);
    for (auto* child : children_)
      child->UpdateChildLayerVisibility(true);
  }

  SetLayer(std::make_unique<crui::Layer>(layer_type));
  layer()->set_delegate(this);
  ///layer()->SetName(GetClassName());

  UpdateParentLayers();
  UpdateLayerVisibility();

  // The new layer needs to be ordered in the layer tree according
  // to the view tree. Children of this layer were added in order
  // in UpdateParentLayers().
  if (parent())
    parent()->ReorderLayers();

  Widget* widget = GetWidget();
  if (widget)
    widget->LayerTreeChanged();

  // Before having its own Layer, this View may have painted in to a Layer owned
  // by an ancestor View. Scheduling a paint on the parent View will erase this
  // View's painting effects on the ancestor View's Layer.
  // (See crbug.com/551492)
  SchedulePaintOnParent();
}

bool View::UpdateParentLayers() {
  // Attach all top-level un-parented layers.
  if (layer()) {
    if (!layer()->parent()) {
      UpdateParentLayer();
      return true;
    }
    // The layers of any child views are already in place, so we can stop
    // iterating here.
    return false;
  }
  bool result = false;
  internal::ScopedChildrenLock lock(this);
  for (auto* child : children_) {
    if (child->UpdateParentLayers())
      result = true;
  }
  return result;
}

void View::OrphanLayers() {
  if (layer()) {
    crui::Layer* parent = layer()->parent();
    if (parent) {
      parent->Remove(layer());
      for (crui::Layer* layer_beneath : layers_beneath_)
        parent->Remove(layer_beneath);
    }

    // The layer belonging to this View has already been orphaned. It is not
    // necessary to orphan the child layers.
    return;
  }
  internal::ScopedChildrenLock lock(this);
  for (auto* child : children_)
    child->OrphanLayers();
}

void View::ReparentLayer(crui::Layer* parent_layer) {
  CR_DCHECK(layer() != parent_layer);
  if (parent_layer) {
    // Adding the main layer can trigger a call to |SnapLayerToPixelBoundary()|.
    // That method assumes layers beneath have already been added. Therefore
    // layers beneath must be added first here. See crbug.com/961212.
    for (crui::Layer* layer_beneath : layers_beneath_)
      parent_layer->Add(layer_beneath);
    parent_layer->Add(layer());
  }
  // Update the layer bounds; this needs to be called after this layer is added
  // to the new parent layer since snapping to pixel boundary will be affected
  // by the layer hierarchy.
  LayerOffsetData offset =
      parent_ ? parent_->CalculateOffsetToAncestorWithLayer(nullptr)
              : LayerOffsetData(layer()->device_scale_factor());
  SetLayerBounds(size(), offset + GetMirroredBounds().OffsetFromOrigin());
  ///layer()->SchedulePaint(GetLocalBounds());
  MoveLayerToParent(layer(), LayerOffsetData(layer()->device_scale_factor()));
}

// Input -----------------------------------------------------------------------

bool View::ProcessMousePressed(const crui::MouseEvent& event) {
  ///int drag_operations =
  ///    (enabled_ && event.IsOnlyLeftMouseButton() &&
  ///     HitTestPoint(event.location())) ?
  ///     GetDragOperations(event.location()) : 0;
  ///ContextMenuController* context_menu_controller =
  ///    event.IsRightMouseButton() ? context_menu_controller_ : nullptr;
  ///View::DragInfo* drag_info = GetDragInfo();

  const bool was_enabled = GetEnabled();
  const bool result = OnMousePressed(event);

  if (!was_enabled)
    return result;

  ///if (event.IsOnlyRightMouseButton() && context_menu_controller &&
  ///    kContextMenuOnMousePress) {
  ///  // Assume that if there is a context menu controller we won't be deleted
  ///  // from mouse pressed.
  ///  if (HitTestPoint(event.location())) {
  ///    gfx::Point location(event.location());
  ///    ConvertPointToScreen(this, &location);
  ///    ShowContextMenu(location, crui::MENU_SOURCE_MOUSE);
  ///    return true;
  ///  }
  ///}
  ///
  ///// WARNING: we may have been deleted, don't use any View variables.
  ///if (drag_operations != crui::DragDropTypes::DRAG_NONE) {
  ///  drag_info->PossibleDrag(event.location());
  ///  return true;
  ///}
  ///return !!context_menu_controller || result;
  return result;
}

void View::ProcessMouseDragged(crui::MouseEvent* event) {
  // Copy the field, that way if we're deleted after drag and drop no harm is
  // done.
  ///ContextMenuController* context_menu_controller = context_menu_controller_;
  ///const bool possible_drag = GetDragInfo()->possible_drag;
  ///if (possible_drag &&
  ///    ExceededDragThreshold(GetDragInfo()->start_pt - event->location()) &&
  ///    (!drag_controller_ ||
  ///     drag_controller_->CanStartDragForView(this, GetDragInfo()->start_pt,
  ///                                           event->location()))) {
  ///  if (DoDrag(*event, GetDragInfo()->start_pt,
  ///             crui::DragDropTypes::DRAG_EVENT_SOURCE_MOUSE)) {
  ///    event->StopPropagation();
  ///    return;
  ///  }
  ///} else {
    if (OnMouseDragged(*event)) {
      event->SetHandled();
      return;
    }
    // Fall through to return value based on context menu controller.
  ///}
  // WARNING: we may have been deleted.
  ///if ((context_menu_controller != nullptr) || possible_drag)
  ///  event->SetHandled();
}

void View::ProcessMouseReleased(const crui::MouseEvent& event) {
  ///if (!kContextMenuOnMousePress && context_menu_controller_ &&
  ///    event.IsOnlyRightMouseButton()) {
  ///  // Assume that if there is a context menu controller we won't be deleted
  ///  // from mouse released.
  ///  gfx::Point location(event.location());
  ///  OnMouseReleased(event);
  ///  if (HitTestPoint(location)) {
  ///    ConvertPointToScreen(this, &location);
  ///    ShowContextMenu(location, crui::MENU_SOURCE_MOUSE);
  ///  }
  ///} else {
    OnMouseReleased(event);
  ///}
  // WARNING: we may have been deleted.
}

// Accelerators ----------------------------------------------------------------

void View::RegisterPendingAccelerators() {
  if (!accelerators_ ||
      registered_accelerator_count_ == accelerators_->size()) {
    // No accelerators are waiting for registration.
    return;
  }

  if (!GetWidget()) {
    // The view is not yet attached to a widget, defer registration until then.
    return;
  }

  accelerator_focus_manager_ = GetFocusManager();
  if (!accelerator_focus_manager_) {
    // Some crash reports seem to show that we may get cases where we have no
    // focus manager (see bug #1291225).  This should never be the case, just
    // making sure we don't crash.
    CR_NOTREACHED();
    return;
  }
  for (std::vector<crui::Accelerator>::const_iterator i(
           accelerators_->begin() + registered_accelerator_count_);
       i != accelerators_->end(); ++i) {
    accelerator_focus_manager_->RegisterAccelerator(
        *i, crui::AcceleratorManager::kNormalPriority, this);
  }
  registered_accelerator_count_ = accelerators_->size();
}

void View::UnregisterAccelerators(bool leave_data_intact) {
  if (!accelerators_)
    return;

  if (GetWidget()) {
    if (accelerator_focus_manager_) {
      accelerator_focus_manager_->UnregisterAccelerators(this);
      accelerator_focus_manager_ = nullptr;
    }
    if (!leave_data_intact) {
      accelerators_->clear();
      accelerators_.reset();
    }
    registered_accelerator_count_ = 0;
  }
}

// Focus -----------------------------------------------------------------------

void View::SetFocusSiblings(View* view, Views::const_iterator pos) {
  // |view| was just inserted at |pos|, so all of these conditions must hold.
  CR_DCHECK(!children_.empty());
  CR_DCHECK(pos != children_.cend());
  CR_DCHECK(view == *pos);

  View* prev = nullptr;
  View* next = nullptr;
  if (children_.size() > 1) {
    if (pos == std::prev(children_.cend())) {
      // |view| was inserted at the end, but the end of the child list may not
      // be the last focusable element. Try to hook in after the last focusable
      // child.
      prev = *std::find_if(children_.cbegin(), pos,
                           [](View* v) { return !v->next_focusable_view_; });
      if (prev == view) {
        // There is a cycle in the focus list. Insert |view| after the last
        // child.
        prev = *std::prev(pos);
      }
      next = prev->next_focusable_view_;
    } else {
      // |view| was inserted somewhere other than the end.  Hook in before the
      // subsequent child.
      next = *std::next(pos);
      prev = next->GetPreviousFocusableView();
    }
  }
  if (prev)
    prev->next_focusable_view_ = view;
  view->previous_focusable_view_ = prev;
  view->next_focusable_view_ = next;
  if (next)
    next->previous_focusable_view_ = view;
}

void View::AdvanceFocusIfNecessary() {
  // Focus should only be advanced if this is the focused view and has become
  // unfocusable. If the view is still focusable or is not focused, we can
  // return early avoiding further unnecessary checks. Focusability check is
  // performed first as it tends to be faster.
  if (IsAccessibilityFocusable() || !HasFocus())
    return;

  FocusManager* focus_manager = GetFocusManager();
  if (focus_manager)
    focus_manager->AdvanceFocusIfNecessary();
}

// System events ---------------------------------------------------------------

void View::PropagateThemeChanged() {
  {
    internal::ScopedChildrenLock lock(this);
    for (auto* child : cr::Reversed(children_))
      child->PropagateThemeChanged();
  }
  OnThemeChanged();
  for (ViewObserver& observer : observers_)
    observer.OnViewThemeChanged(this);
}

void View::PropagateDeviceScaleFactorChanged(float old_device_scale_factor,
                                             float new_device_scale_factor) {
  {
    internal::ScopedChildrenLock lock(this);
    for (auto* child : cr::Reversed(children_)) {
      child->PropagateDeviceScaleFactorChanged(old_device_scale_factor,
                                               new_device_scale_factor);
    }
  }

  // If the view is drawing to the layer, OnDeviceScaleFactorChanged() is called
  // through LayerDelegate callback.
  ///if (!layer())
    OnDeviceScaleFactorChanged(old_device_scale_factor,
                               new_device_scale_factor);
}

// Tooltips --------------------------------------------------------------------

void View::UpdateTooltip() {
  Widget* widget = GetWidget();
  // TODO(beng): The TooltipManager nullptr check can be removed when we
  //             consolidate Init() methods and make views_unittests Init() all
  //             Widgets that it uses.
  if (widget && widget->GetTooltipManager())
    widget->GetTooltipManager()->UpdateTooltip();
}

// Drag and drop ---------------------------------------------------------------

bool View::DoDrag(const crui::LocatedEvent& event,
                  const gfx::Point& press_pt,
                  crui::DragDropTypes::DragEventSource source) {
  int drag_operations = GetDragOperations(press_pt);
  if (drag_operations == crui::DragDropTypes::DRAG_NONE)
    return false;

  Widget* widget = GetWidget();
  // We should only start a drag from an event, implying we have a widget.
  CR_DCHECK(widget);

  // Don't attempt to start a drag while in the process of dragging. This is
  // especially important on X where we can get multiple mouse move events when
  // we start the drag.
  if (widget->dragged_view())
    return false;

  std::unique_ptr<OSExchangeData> data(std::make_unique<OSExchangeData>());
  WriteDragData(press_pt, data.get());

  // Message the RootView to do the drag and drop. That way if we're removed
  // the RootView can detect it and avoid calling us back.
  gfx::Point widget_location(event.location());
  ConvertPointToWidget(this, &widget_location);
  widget->RunShellDrag(this, std::move(data), widget_location, drag_operations,
                       source);
  // WARNING: we may have been deleted.
  return true;
}

DEFINE_ENUM_CONVERTERS(View::FocusBehavior,
                       {View::FocusBehavior::ACCESSIBLE_ONLY,
                        cr::ASCIIToUTF16("ACCESSIBLE_ONLY")},
                       {View::FocusBehavior::ALWAYS,
                        cr::ASCIIToUTF16("ALWAYS")},
                       {View::FocusBehavior::NEVER,
                        cr::ASCIIToUTF16("NEVER")})

// This block requires the existence of METADATA_HEADER(View) in the class
// declaration for View.
BEGIN_METADATA(View)
ADD_READONLY_PROPERTY_METADATA(View, const char*, ClassName)
ADD_PROPERTY_METADATA(View, bool, Enabled)
ADD_PROPERTY_METADATA(View, View::FocusBehavior, FocusBehavior)
ADD_PROPERTY_METADATA(View, int, Group)
ADD_PROPERTY_METADATA(View, int, ID)
ADD_READONLY_PROPERTY_METADATA(View, gfx::Size, MaximumSize)
ADD_READONLY_PROPERTY_METADATA(View, gfx::Size, MinimumSize)
ADD_PROPERTY_METADATA(View, bool, Visible)
END_METADATA()

}  // namespace views
}  // namespace crui
