// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_GEOMETRY_ANGLE_CONVERSIONS_H_
#define UI_GFX_GEOMETRY_ANGLE_CONVERSIONS_H_

#include "crbase/numerics/math_constants.h"
#include "crui/base/ui_export.h"

namespace crui {
namespace gfx {

CRUI_EXPORT constexpr double DegToRad(double deg) {
  return deg * cr::kPiDouble / 180.0;
}
CRUI_EXPORT constexpr float DegToRad(float deg) {
  return deg * cr::kPiFloat / 180.0f;
}

CRUI_EXPORT constexpr double RadToDeg(double rad) {
  return rad * 180.0 / cr::kPiDouble;
}
CRUI_EXPORT constexpr float RadToDeg(float rad) {
  return rad * 180.0f / cr::kPiFloat;
}

}  // namespace gfx
}  // namespace crui

#endif  // UI_GFX_GEOMETRY_ANGLE_CONVERSIONS_H_
