// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_GEOMETRY_POINT_CONVERSIONS_H_
#define UI_GFX_GEOMETRY_POINT_CONVERSIONS_H_

#include "crui/gfx/geometry/point.h"
#include "crui/gfx/geometry/point_f.h"

namespace crui {
namespace gfx {

// Returns a Point with each component from the input PointF floored.
CRUI_EXPORT Point ToFlooredPoint(const PointF& point);

// Returns a Point with each component from the input PointF ceiled.
CRUI_EXPORT Point ToCeiledPoint(const PointF& point);

// Returns a Point with each component from the input PointF rounded.
CRUI_EXPORT Point ToRoundedPoint(const PointF& point);

}  // namespace gfx
}  // namespace crui

#endif  // UI_GFX_GEOMETRY_POINT_CONVERSIONS_H_
