// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/compositor/layer_tree_owner.h"

#include "crui/compositor/layer.h"

namespace crui {

namespace {

// Deletes |layer| and all its descendants.
void DeepDeleteLayers(Layer* layer) {
  std::vector<Layer*> children = layer->children();
  for (std::vector<Layer*>::const_iterator it = children.begin();
       it != children.end();
       ++it) {
    Layer* child = *it;
    DeepDeleteLayers(child);
  }
  delete layer;
}

}  // namespace

LayerTreeOwner::LayerTreeOwner(std::unique_ptr<Layer> root)
    : root_(root.release()) {}

LayerTreeOwner::~LayerTreeOwner() {
  if (root_)
    DeepDeleteLayers(root_);
}

}  // namespace crui
