// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/gfx/geometry/insets.h"

#include "crbase/strings/stringprintf.h"
#include "crui/gfx/geometry/vector2d.h"

namespace crui {
namespace gfx {

std::string Insets::ToString() const {
  // Print members in the same order of the constructor parameters.
  return cr::StringPrintf("%d,%d,%d,%d", top(),  left(), bottom(), right());
}

Insets Insets::Offset(const gfx::Vector2d& vector) const {
  return gfx::Insets(cr::ClampAdd(top(), vector.y()),
                     cr::ClampAdd(left(), vector.x()),
                     cr::ClampSub(bottom(), vector.y()),
                     cr::ClampSub(right(), vector.x()));
}

}  // namespace gfx
}  // namespace crui
