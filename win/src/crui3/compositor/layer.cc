// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/compositor/layer.h"

#include "crbase/containers/optional.h"

namespace {

const crui::Layer* GetRoot(const crui::Layer* layer) {
  while (layer->parent())
    layer = layer->parent();
  return layer;
}

}  // namespace

namespace crui {

Layer::Layer(LayerType type) 
    : type_(type),
      parent_(nullptr),
      visible_(true), 
      fills_bounds_opaquely_(true),
      fills_bounds_completely_(false),
      delegate_(nullptr),
      owner_(nullptr),
      opacity_(1.0f) {
}

Layer::~Layer() {

}

std::unique_ptr<Layer> Layer::Clone() const {
  auto clone = std::make_unique<Layer>(type_);

  // Background filters.
  ///clone->SetBackgroundBlur(background_blur_sigma_);
  ///clone->SetBackgroundZoom(zoom_, zoom_inset_);

  // Filters.
  ///clone->SetLayerSaturation(layer_saturation_);
  ///clone->SetLayerBrightness(GetTargetBrightness());
  ///clone->SetLayerGrayscale(GetTargetGrayscale());
  ///clone->SetLayerInverted(layer_inverted_);
  ///clone->SetLayerBlur(layer_blur_sigma_);
  ///if (alpha_shape_)
  ///  clone->SetAlphaShape(std::make_unique<ShapeRects>(*alpha_shape_));
  ///
  ///// cc::Layer state.
  ///if (surface_layer_) {
  ///  clone->SetShowSurface(surface_layer_->surface_id(), frame_size_in_dip_,
  ///                        surface_layer_->background_color(),
  ///                        surface_layer_->deadline_in_frames()
  ///                            ? cc::DeadlinePolicy::UseSpecifiedDeadline(
  ///                                  *surface_layer_->deadline_in_frames())
  ///                            : cc::DeadlinePolicy::UseDefaultDeadline(),
  ///                        surface_layer_->stretch_content_to_fill_bounds());
  ///  if (surface_layer_->oldest_acceptable_fallback())
  ///    clone->SetOldestAcceptableFallback(
  ///        *surface_layer_->oldest_acceptable_fallback());
  ///} else if (type_ == LAYER_SOLID_COLOR) {
  ///  clone->SetColor(GetTargetColor());
  ///}

  clone->SetTransform(GetTargetTransform());
  clone->SetBounds(bounds_);
  ///if (subpixel_position_offset_->has_explicit_subpixel_offset())
  ///  clone->SetSubpixelPositionOffset(GetSubpixelOffset());
  ///clone->SetMasksToBounds(GetMasksToBounds());
  clone->SetOpacity(GetTargetOpacity());
  clone->SetVisible(GetTargetVisibility());
  ///clone->SetAcceptEvents(accept_events());
  clone->SetFillsBoundsOpaquely(fills_bounds_opaquely_);
  clone->SetFillsBoundsCompletely(fills_bounds_completely_);
  ///clone->SetRoundedCornerRadius(rounded_corner_radii());
  ///clone->SetIsFastRoundedCorner(is_fast_rounded_corner());
  ///clone->SetName(name_);

  return clone;
}

void Layer::AddObserver(LayerObserver* observer) {
  observer_list_.AddObserver(observer);
}

void Layer::RemoveObserver(LayerObserver* observer) {
  observer_list_.RemoveObserver(observer);
}

void Layer::Add(Layer* child) {
  ///CR_DCHECK(!child->compositor_);
  if (child->parent_)
    child->parent_->Remove(child);
  child->parent_ = this;
  children_.push_back(child);
  ///cc_layer_->AddChild(child->cc_layer_);
  child->OnDeviceScaleFactorChanged(device_scale_factor_);
  ///Compositor* compositor = GetCompositor();
  ///if (compositor)
  ///  child->SetCompositorForAnimatorsInTree(compositor);
}

void Layer::Remove(Layer* child) {
  // Current bounds are used to calculate offsets when layers are reparented.
  // Stop (and complete) an ongoing animation to update the bounds immediately.
  ///LayerAnimator* child_animator = child->animator_.get();
  ///if (child_animator)
  ///  child_animator->StopAnimatingProperty(ui::LayerAnimationElement::BOUNDS);

  ///Compositor* compositor = GetCompositor();
  ///if (compositor)
  ///  child->ResetCompositorForAnimatorsInTree(compositor);

  std::vector<Layer*>::iterator i =
    std::find(children_.begin(), children_.end(), child);
  CR_DCHECK(i != children_.end());
  children_.erase(i);
  child->parent_ = NULL;
  ///child->cc_layer_->RemoveFromParent();
}

void Layer::StackAtTop(Layer* child) {
  if (children_.size() <= 1 || child == children_.back())
    return;  // Already in front.
  StackAbove(child, children_.back());
}

void Layer::StackAbove(Layer* child, Layer* other) {
  StackRelativeTo(child, other, true);
}

void Layer::StackAtBottom(Layer* child) {
  if (children_.size() <= 1 || child == children_.front())
    return;  // Already on bottom.
  StackBelow(child, children_.front());
}

void Layer::StackBelow(Layer* child, Layer* other) {
  StackRelativeTo(child, other, false);
}

void Layer::SetTransform(const gfx::Transform& transform) {
  transform_ = transform;
}

const gfx::Transform& Layer::transform() const {
  return transform_;
}

gfx::Transform Layer::GetTargetTransform() const {
  return transform_;
}

void Layer::SetBounds(const gfx::Rect& bounds) {
  bounds_ = bounds;
}

gfx::Rect Layer::GetTargetBounds() const {
  return bounds_;
}

float Layer::opacity() const {
  return opacity_;
}

void Layer::SetOpacity(float opacity) {
  opacity_ = opacity;
}

float Layer::GetTargetOpacity() const {
  return opacity_;
}

void Layer::SetVisible(bool visible) {
  visible_ = visible;
}

bool Layer::GetTargetVisibility() const {
  return visible_;
}

bool Layer::IsDrawn() const {
  const Layer* layer = this;
  while (layer && layer->visible_)
    layer = layer->parent_;
  return layer == NULL;
}

// static 
void Layer::ConvertPointToLayer(const Layer* source,
                                const Layer* target,
                                gfx::PointF* point) {
  if (source == target)
    return;

  const Layer* root_layer = GetRoot(source);
  CR_CHECK(root_layer == GetRoot(target));

  if (source != root_layer)
    source->ConvertPointForAncestor(root_layer, point);
  if (target != root_layer)
    target->ConvertPointFromAncestor(root_layer, point);
}

bool Layer::GetTargetTransformRelativeTo(const Layer* ancestor,
                                         gfx::Transform* transform) const {
  const Layer* p = this;
  for (; p && p != ancestor; p = p->parent()) {
    gfx::Transform translation;
    translation.Translate(static_cast<float>(p->bounds().x()),
                          static_cast<float>(p->bounds().y()));
    // Use target transform so that result will be correct once animation is
    // finished.
    if (!p->GetTargetTransform().IsIdentity())
      transform->PostConcat(p->GetTargetTransform());
    transform->PostConcat(translation);
  }
  return p == ancestor;
}

void Layer::SetFillsBoundsOpaquely(bool fills_bounds_opaquely) {
  if (fills_bounds_opaquely_ == fills_bounds_opaquely)
    return;

  fills_bounds_opaquely_ = fills_bounds_opaquely;

  ///cc_layer_->SetContentsOpaque(fills_bounds_opaquely);

  if (delegate_)
    delegate_->OnLayerFillsBoundsOpaquelyChanged();
}

void Layer::SetFillsBoundsCompletely(bool fills_bounds_completely) {
  fills_bounds_completely_ = fills_bounds_completely;
}

void Layer::OnDeviceScaleFactorChanged(float device_scale_factor) {
    if (device_scale_factor_ == device_scale_factor)
    return;

  cr::WeakPtr<Layer> weak_this = weak_ptr_factory_.GetWeakPtr();
  
  ///// Some animation observers may mutate the tree (e.g. destroy the layer,
  ///// change ancestor/sibling z-order etc) when the animation ends. This break
  ///// the tree traversal and could lead to a crash. Collect all descendants (and
  ///// their mask layers) in a flattened WeakPtr list at the root level then stop
  ///// animations to let potential tree mutations happen before traversing the
  ///// tree. See https://crbug.com/1037852.
  ///const bool is_root_layer = !parent();
  ///if (is_root_layer) {
  ///  std::vector<base::WeakPtr<Layer>> flattened;
  ///  GetFlattenedWeakList(&flattened);
  ///  for (auto& weak_layer : flattened) {
  ///    // Skip if layer is gone or not animating.
  ///    if (!weak_layer || !weak_layer->animator_)
  ///      continue;
  ///
  ///    weak_layer->animator_->StopAnimatingProperty(
  ///        LayerAnimationElement::TRANSFORM);
  ///
  ///    // Do not proceed if the root layer was destroyed due to an animation
  ///    // observer.
  ///    if (!weak_this)
  ///      return;
  ///  }
  ///}

  const float old_device_scale_factor = device_scale_factor_;
  device_scale_factor_ = device_scale_factor;
  ///RecomputeDrawsContentAndUVRect();
  ///RecomputePosition();
  ///if (nine_patch_layer_) {
  ///  if (!nine_patch_layer_image_.isNull())
  ///    UpdateNinePatchLayerImage(nine_patch_layer_image_);
  ///  UpdateNinePatchLayerAperture(nine_patch_layer_aperture_);
  ///}
  ///SchedulePaint(gfx::Rect(bounds_.size()));
  if (delegate_) {
    delegate_->OnDeviceScaleFactorChanged(old_device_scale_factor,
                                          device_scale_factor);
  }
  for (auto* child : children_) {
    child->OnDeviceScaleFactorChanged(device_scale_factor);

    // A child layer may have triggered a delegate or an observer to delete
    // |this| layer. In which case return early to avoid crash.
    if (!weak_this)
      return;
  }
  ///if (layer_mask_)
  ///  layer_mask_->OnDeviceScaleFactorChanged(device_scale_factor);
}

void Layer::StackRelativeTo(Layer* child, Layer* other, bool above) {
  CR_DCHECK(child != other);
  CR_DCHECK(this == child->parent());
  CR_DCHECK(this == other->parent());

  const size_t child_i =
    std::find(children_.begin(), children_.end(), child) - children_.begin();
  const size_t other_i =
    std::find(children_.begin(), children_.end(), other) - children_.begin();
  if ((above && child_i == other_i + 1) || (!above && child_i + 1 == other_i))
    return;

  const size_t dest_i =
    above ?
    (child_i < other_i ? other_i : other_i + 1) :
    (child_i < other_i ? other_i - 1 : other_i);

  children_.erase(children_.begin() + child_i);
  children_.insert(children_.begin() + dest_i, child);

  ///child->cc_layer_->RemoveFromParent();
  ///cc_layer_->InsertChild(child->cc_layer_, dest_i);
}

bool Layer::ConvertPointForAncestor(const Layer* ancestor, 
                                    gfx::PointF* point) const {
  gfx::Transform transform;
  if (!GetTargetTransformRelativeTo(ancestor, &transform))
    return false;

  *point = transform.MapPoint(*point);
  return true;
}

bool Layer::ConvertPointFromAncestor(const Layer* ancestor, 
                                     gfx::PointF* point) const {
  gfx::Transform transform;
  if (!GetTargetTransformRelativeTo(ancestor, &transform)) {
    return false;
  }
  const cr::Optional<gfx::PointF> transformed_point =
      transform.InverseMapPoint(*point);
  if (!transformed_point.has_value())
    return false;

  *point = transformed_point.value();
  return true;
}

}  // namespace crui