// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/gfx/geometry/rounded_corners_f.h"

#include "crbase/strings/stringprintf.h"

namespace crui {
namespace gfx {

std::string RoundedCornersF::ToString() const {
  // Print members in the same order of the constructor parameters.
  return cr::StringPrintf("%f,%f,%f,%f", upper_left_, upper_right_,
                          lower_right_, lower_left_);
}

}  // namespace gfx
}  // namespace crui
