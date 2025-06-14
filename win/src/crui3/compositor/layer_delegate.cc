// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/compositor/layer_delegate.h"

namespace crui {

void LayerDelegate::OnLayerBoundsChanged(const gfx::Rect& old_bounds,
                                         PropertyChangeReason reason) {}

void LayerDelegate::OnLayerTransformed(const gfx::Transform& old_transform,
                                       PropertyChangeReason reason) {}

void LayerDelegate::OnLayerOpacityChanged(PropertyChangeReason reason) {}

void LayerDelegate::OnLayerAlphaShapeChanged() {}

void LayerDelegate::OnLayerFillsBoundsOpaquelyChanged() {}

void LayerDelegate::UpdateVisualState() {}

}  // namespace crui
