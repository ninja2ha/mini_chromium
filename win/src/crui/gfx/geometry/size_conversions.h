// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_GEOMETRY_SIZE_CONVERSIONS_H_
#define UI_GFX_GEOMETRY_SIZE_CONVERSIONS_H_

#include "crui/gfx/geometry/size.h"
#include "crui/gfx/geometry/size_f.h"

namespace crui {
namespace gfx {

// Returns a Size with each component from the input SizeF floored.
CRUI_EXPORT Size ToFlooredSize(const SizeF& size);

// Returns a Size with each component from the input SizeF ceiled.
CRUI_EXPORT Size ToCeiledSize(const SizeF& size);

// Returns a Size with each component from the input SizeF rounded.
CRUI_EXPORT Size ToRoundedSize(const SizeF& size);

}  // namespace gfx
}  // namespace crui

#endif  // UI_GFX_GEOMETRY_SIZE_CONVERSIONS_H_
