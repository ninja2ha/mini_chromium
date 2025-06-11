// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKIA_EXT_SKIA_UTILS_BASE_H_
#define SKIA_EXT_SKIA_UTILS_BASE_H_

#include "third_party/skia/include/core/SkSurfaceProps.h"

namespace skia {

// Determine the default pixel geometry (for LCD) by querying the font host
SK_API SkPixelGeometry ComputeDefaultPixelGeometry();

}  // namespace skia

#endif  // SKIA_EXT_SKIA_UTILS_BASE_H_

