// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/gfx/geometry/insets_f.h"

#include "crbase/strings/stringprintf.h"

namespace crui {
namespace gfx {

std::string InsetsF::ToString() const {
  // Print members in the same order of the constructor parameters.
  return cr::StringPrintf("%f,%f,%f,%f", top(),  left(), bottom(), right());
}

}  // namespace gfx
}  // namespace crui
