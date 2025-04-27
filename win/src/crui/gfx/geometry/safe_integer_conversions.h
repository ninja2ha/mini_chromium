// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_GEOMETRY_SAFE_INTEGER_CONVERSIONS_H_
#define UI_GFX_GEOMETRY_SAFE_INTEGER_CONVERSIONS_H_

#include <cmath>
#include <limits>

#include "crbase/numerics/safe_conversions.h"

namespace crui {
namespace gfx {

inline int ToFlooredInt(float value) {
  return cr::saturated_cast<int>(std::floor(value));
}

inline int ToCeiledInt(float value) {
  return cr::saturated_cast<int>(std::ceil(value));
}

inline int ToFlooredInt(double value) {
  return cr::saturated_cast<int>(std::floor(value));
}

inline int ToCeiledInt(double value) {
  return cr::saturated_cast<int>(std::ceil(value));
}

inline int ToRoundedInt(float value) {
  float rounded;
  if (value >= 0.0f)
    rounded = std::floor(value + 0.5f);
  else
    rounded = std::ceil(value - 0.5f);
  return cr::saturated_cast<int>(rounded);
}

inline int ToRoundedInt(double value) {
  double rounded;
  if (value >= 0.0)
    rounded = std::floor(value + 0.5);
  else
    rounded = std::ceil(value - 0.5);
  return cr::saturated_cast<int>(rounded);
}

inline bool IsExpressibleAsInt(float value) {
  if (value != value)
    return false; // no int NaN.
  if (value > std::numeric_limits<int>::max())
    return false;
  if (value < std::numeric_limits<int>::min())
    return false;
  return true;
}

}  // namespace gfx
}  // namespace crui

#endif  // UI_GFX_GEOMETRY_SAFE_INTEGER_CONVERSIONS_H_
