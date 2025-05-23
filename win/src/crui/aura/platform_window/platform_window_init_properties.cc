// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/aura/platform_window/platform_window_init_properties.h"

namespace crui {

PlatformWindowInitProperties::PlatformWindowInitProperties() = default;

PlatformWindowInitProperties::PlatformWindowInitProperties(
    const gfx::Rect& bounds)
    : bounds(bounds) {}

PlatformWindowInitProperties::PlatformWindowInitProperties(
    PlatformWindowInitProperties&& props) = default;

PlatformWindowInitProperties::~PlatformWindowInitProperties() = default;

}  // namespace crui
