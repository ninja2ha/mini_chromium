// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_GEOMETRY_RECT_CONVERSIONS_H_
#define UI_GFX_GEOMETRY_RECT_CONVERSIONS_H_

#include "crui/gfx/geometry/rect.h"
#include "crui/gfx/geometry/rect_f.h"

namespace crui {
namespace gfx {

// Returns the smallest Rect that encloses the given RectF.
CRUI_EXPORT Rect ToEnclosingRect(const RectF& rect);

// Similar to ToEnclosingRect(), but for each edge, if the distance between the
// edge and the nearest integer grid is smaller than |error|, the edge is
// snapped to the integer grid. Unlike ToNearestRect() which only accepts
// integer rect with or without floating point error, this function also accepts
// non-integer rect.
CRUI_EXPORT Rect ToEnclosingRectIgnoringError(const RectF& rect,
                                              float error);

// Returns the largest Rect that is enclosed by the given RectF.
CRUI_EXPORT Rect ToEnclosedRect(const RectF& rect);

// Similar to ToEnclosedRect(), but for each edge, if the distance between the
// edge and the nearest integer grid is smaller than |error|, the edge is
// snapped to the integer grid. Unlike ToNearestRect() which only accepts
// integer rect with or without floating point error, this function also accepts
// non-integer rect.
CRUI_EXPORT Rect ToEnclosedRectIgnoringError(const RectF& rect,
                                             float error);

// Returns the Rect after snapping the corners of the RectF to an integer grid.
// This should only be used when the RectF you provide is expected to be an
// integer rect with floating point error. If it is an arbitrary RectF, then
// you should use a different method.
CRUI_EXPORT Rect ToNearestRect(const RectF& rect);

// Returns true if the Rect produced after snapping the corners of the RectF
// to an integer grid is withing |distance|.
CRUI_EXPORT bool IsNearestRectWithinDistance(const gfx::RectF& rect,
                                             float distance);

// Returns a Rect obtained by flooring the values of the given RectF.
// Please prefer the previous two functions in new code.
CRUI_EXPORT Rect ToFlooredRectDeprecated(const RectF& rect);

}  // namespace gfx
}  // namespace crui

#endif  // UI_GFX_GEOMETRY_RECT_CONVERSIONS_H_
