// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_COMPOSITOR_LAYER_OBSERVER_H_
#define UI_COMPOSITOR_LAYER_OBSERVER_H_

#include "crui/base/ui_export.h"

namespace crui {

class Layer;

class CRUI_EXPORT LayerObserver {
 public:
  virtual void LayerDestroyed(Layer* layer) {}

 protected:
  virtual ~LayerObserver() {}
};

}  // namespace crui

#endif  // UI_COMPOSITOR_LAYER_OBSERVER_H_
