// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_GEOMETRY_DECOMPOSED_TRANSFORM_H_
#define UI_GFX_GEOMETRY_DECOMPOSED_TRANSFORM_H_

#include <array>

#include "crui/base/ui_export.h"
#include "crui/gfx/geometry/quaternion.h"
#include "crui/gfx/geometry/vector3d_f.h"
#include "crui/gfx/geometry/vector4d_f.h"

namespace crui {
namespace gfx {

// Contains the components of a factored transform. These components may be
// blended and recomposed.
struct CRUI_EXPORT DecomposedTransform {
  // The default constructor initializes the components in such a way that
  // will compose the identity transform.
  std::array<double, 3u> translate = { 0, 0, 0 };
  std::array<double, 3u> scale = { 1, 1, 1 };
  std::array<double, 3u> skew = { 0, 0, 0 };
  std::array<double, 4u> perspective = { 0, 0, 0, 1 };
  Quaternion quaternion;

  std::string ToString() const;
};

}  // namespace gfx
}  // namespace crui

#endif  // UI_GFX_GEOMETRY_DECOMPOSED_TRANSFORM_H_