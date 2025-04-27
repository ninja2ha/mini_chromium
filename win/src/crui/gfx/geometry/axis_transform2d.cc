// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/gfx/geometry/axis_transform2d.h"

#include "crbase/strings/stringprintf.h"

namespace crui {
namespace gfx {

std::string AxisTransform2d::ToString() const {
  return cr::StringPrintf("[%f, %s]", scale_,
                          translation_.ToString().c_str());
}

}  // namespace gfx
}  // namespace crui