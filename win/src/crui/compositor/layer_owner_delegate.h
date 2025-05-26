// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_COMPOSITOR_LAYER_OWNER_DELEGATE_H_
#define UI_COMPOSITOR_LAYER_OWNER_DELEGATE_H_

#include "crui/base/ui_export.h"

namespace crui {
class Layer;

// Called from RecreateLayer() after the new layer was created. old_layer is
// the layer that will be returned to the caller of RecreateLayer, new_layer
// will be the layer now used.
class CRUI_EXPORT LayerOwnerDelegate {
 public:
  virtual void OnLayerRecreated(Layer* old_layer, Layer* new_layer) = 0;

 protected:
  virtual ~LayerOwnerDelegate() {}
};

}  // namespace crui

#endif  // UI_COMPOSITOR_LAYER_OWNER_DELEGATE_H_
