// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/gfx/geometry/scroll_offset.h"

#include "crbase/strings/stringprintf.h"

namespace crui {
namespace gfx {

std::string ScrollOffset::ToString() const {
  return cr::StringPrintf("[%f %f]", x_, y_);
}

}  // namespace gfx
}  // namespace crui
