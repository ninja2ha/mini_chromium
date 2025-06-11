// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/gfx/geometry/range.h"

#include <inttypes.h>

#include <algorithm>

#include "crbase/logging.h"
#include "crbase/strings/stringprintf.h"

namespace crui {
namespace gfx {

std::string Range::ToString() const {
  return cr::StringPrintf("{%" PRIu32 ",%" PRIu32 "}", start(), end());
}

}  // namespace gfx
}  // namespace crui
