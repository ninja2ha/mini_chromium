// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_COMPOSITOR_LAYER_H_
#define UI_COMPOSITOR_LAYER_H_

#include "crbase/memory/weak_ptr.h"
#include "crbase/observer_list.h"
#include "crui/gfx/geometry/point.h"
#include "crui/gfx/geometry/point3_f.h"
#include "crui/gfx/geometry/rect.h"
#include "crui/gfx/geometry/transform.h"
#include "crui/compositor/layer_type.h"
#include "crui/compositor/layer_observer.h"
#include "crui/compositor/layer_delegate.h"
#include "crui/base/ui_export.h"

namespace crui {
class LayerOwner;

class CRUI_EXPORT Layer {
 public:
   explicit Layer(LayerType type = LAYER_TEXTURED);
  virtual ~Layer();

  // Note that only solid color and surface content is copied.
  std::unique_ptr<Layer> Clone() const;

  LayerDelegate* delegate() { return delegate_; }
  void set_delegate(LayerDelegate* delegate) { delegate_ = delegate; }

  LayerOwner* owner() { return owner_; }

  void AddObserver(LayerObserver* observer);
  void RemoveObserver(LayerObserver* observer);

  // Adds a new Layer to this Layer.
  void Add(Layer* child);

  // Removes a Layer from this Layer.
  void Remove(Layer* child);

  // Stacks |child| above all other children.
  void StackAtTop(Layer* child);

  // Stacks |child| directly above |other|.  Both must be children of this
  // layer.  Note that if |child| is initially stacked even higher, calling this
  // method will result in |child| being lowered in the stacking order.
  void StackAbove(Layer* child, Layer* other);

  // Stacks |child| below all other children.
  void StackAtBottom(Layer* child);

  // Stacks |child| directly below |other|.  Both must be children of this
  // layer.
  void StackBelow(Layer* child, Layer* other);

  // Returns the child Layers.
  const std::vector<Layer*>& children() const { return children_; }

  // The parent.
  const Layer* parent() const { return parent_; }
  Layer* parent() { return parent_; }

  LayerType type() const { return type_; }

  // The transform, relative to the parent.
  void SetTransform(const gfx::Transform& transform);
  const gfx::Transform& transform() const;

  // Return the target transform if animator is running, or the current
  // transform otherwise.
  gfx::Transform GetTargetTransform() const;

  // The bounds, relative to the parent.
  void SetBounds(const gfx::Rect& bounds);
  const gfx::Rect& bounds() const { return bounds_; }
  const gfx::Size& size() const { return bounds_.size(); }

  // Return the target bounds if animator is running, or the current bounds
  // otherwise.
  gfx::Rect GetTargetBounds() const;

  // The opacity of the layer. The opacity is applied to each pixel of the
  // texture (resulting alpha = opacity * alpha).
  float opacity() const;
  void SetOpacity(float opacity);

  // Return the target opacity if animator is running, or the current opacity
  // otherwise.
  float GetTargetOpacity() const;

  // Sets the visibility of the Layer. A Layer may be visible but not
  // drawn. This happens if any ancestor of a Layer is not visible.
  void SetVisible(bool visible);
  bool visible() const { return visible_; }

  // Returns the target visibility if the animator is running. Otherwise, it
  // returns the current visibility.
  bool GetTargetVisibility() const;

  // Returns true if this Layer is drawn. A Layer is drawn only if all ancestors
  // are visible.
  bool IsDrawn() const;

  // Converts a point from the coordinates of |source| to the coordinates of
  // |target|. Necessarily, |source| and |target| must inhabit the same Layer
  // tree.
  static void ConvertPointToLayer(const Layer* source,
                                  const Layer* target,
                                  gfx::PointF* point);

  // Converts a transform to be relative to the given |ancestor|. Returns
  // whether success (that is, whether the given ancestor was really an
  // ancestor of this layer).
  bool GetTargetTransformRelativeTo(const Layer* ancestor,
                                    gfx::Transform* transform) const;

  // Note: Setting a layer non-opaque has significant performance impact,
  // especially on low-end Chrome OS devices. Please ensure you are not
  // adding unnecessary overdraw. When in doubt, talk to the graphics team.
  void SetFillsBoundsOpaquely(bool fills_bounds_opaquely);
  bool fills_bounds_opaquely() const { return fills_bounds_opaquely_; }

  // Set to true if this layer always paints completely within its bounds. If so
  // we can omit an unnecessary clear, even if the layer is transparent.
  void SetFillsBoundsCompletely(bool fills_bounds_completely);

  // Notifies the layer that the device scale factor has changed.
  void OnDeviceScaleFactorChanged(float device_scale_factor);

  float device_scale_factor() const { return device_scale_factor_; }

 private:
  friend class LayerOwner;

  // Stacks |child| above or below |other|.  Helper method for StackAbove() and
  // StackBelow().
  void StackRelativeTo(Layer* child, Layer* other, bool above);

  bool ConvertPointForAncestor(const Layer* ancestor, 
                               gfx::PointF* point) const;
  bool ConvertPointFromAncestor(const Layer* ancestor, 
                                gfx::PointF* point) const;

  LayerType type_;
  LayerDelegate* delegate_;

  ///cr::ObserverList<LayerObserver>::Unchecked observer_list_;
  cr::ObserverList<LayerObserver> observer_list_;

  Layer* parent_;

  // This layer's children, in bottom-to-top stacking order.
  std::vector<Layer*> children_;

  gfx::Rect bounds_;
  bool visible_;
  float opacity_;
  gfx::Transform transform_;

  // See SetFillsBoundsOpaquely(). Defaults to true.
  bool fills_bounds_opaquely_;

  bool fills_bounds_completely_;

  // A cached copy of |Compositor::device_scale_factor()|.
  float device_scale_factor_;

  LayerOwner* owner_;
  cr::WeakPtrFactory<Layer> weak_ptr_factory_{this};
};

}  // namespace crui

#endif  // UI_COMPOSITOR_LAYER_H_