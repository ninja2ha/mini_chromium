// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_COMPOSITOR_PROPERTY_CHANGE_REASON_H_
#define UI_COMPOSITOR_PROPERTY_CHANGE_REASON_H_

namespace crui {

enum class PropertyChangeReason {
  // The property value was set by an animation.
  FROM_ANIMATION,
  // The property value was set directly, without an animation.
  NOT_FROM_ANIMATION,
};

}  // namespace crui

#endif  // UI_COMPOSITOR_PROPERTY_CHANGE_REASON_H_
