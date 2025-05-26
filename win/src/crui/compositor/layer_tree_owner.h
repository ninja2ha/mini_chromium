// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_COMPOSITOR_LAYER_TREE_OWNER_H_
#define UI_COMPOSITOR_LAYER_TREE_OWNER_H_

#include <memory>

#include "crbase/compiler_specific.h"
#include "crui/base/ui_export.h"

namespace crui {

class Layer;

// Scoping object that owns a Layer and all its descendants.
class CRUI_EXPORT LayerTreeOwner {
 public:
  LayerTreeOwner(const LayerTreeOwner&) = delete;
  LayerTreeOwner& operator=(const LayerTreeOwner&) = delete;

  explicit LayerTreeOwner(std::unique_ptr<Layer> root);
  ~LayerTreeOwner();

  Layer* release() CR_WARN_UNUSED_RESULT {
    Layer* root = root_;
    root_ = NULL;
    return root;
  }

  Layer* root() { return root_; }
  const Layer* root() const { return root_; }

 private:
  Layer* root_;
};

}  // namespace crui

#endif  // UI_COMPOSITOR_LAYER_TREE_OWNER_H_
