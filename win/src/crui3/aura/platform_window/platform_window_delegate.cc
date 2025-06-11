// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/platform_window/platform_window_delegate.h"

#include "crui/gfx/geometry/size.h"

namespace crui {

PlatformWindowDelegate::PlatformWindowDelegate() = default;

PlatformWindowDelegate::~PlatformWindowDelegate() = default;

cr::Optional<gfx::Size> PlatformWindowDelegate::GetMinimumSizeForWindow() {
  return cr::nullopt;
}

cr::Optional<gfx::Size> PlatformWindowDelegate::GetMaximumSizeForWindow() {
  return cr::nullopt;
}

}  // namespace crui
