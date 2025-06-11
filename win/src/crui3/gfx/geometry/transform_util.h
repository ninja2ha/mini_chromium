// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_GEOMETRY_TRANSFORM_UTIL_H_
#define UI_GFX_GEOMETRY_TRANSFORM_UTIL_H_

#include "crbase/containers/optional.h"
#include "crui/base/ui_export.h"
#include "crui/gfx/geometry/axis_transform2d.h"
#include "crui/gfx/geometry/decomposed_transform.h"
#include "crui/gfx/geometry/point.h"
#include "crui/gfx/geometry/transform.h"

namespace crui {
namespace gfx {

class RectF;

// Returns a scale transform at |anchor| point.
CRUI_EXPORT
Transform GetScaleTransform(const Point& anchor, float scale);

// Interpolates the decomposed components |to| with |from| using the
// routines described in
// https://www.w3.org/TR/css-transforms-2/#interpolation-of-decomposed-3d-matrix-values
// |progress| is in the range [0, 1]. If 0 we will return |from|, if 1, we will
// return |to|.
CRUI_EXPORT
DecomposedTransform BlendDecomposedTransforms(const DecomposedTransform& to,
                                              const DecomposedTransform& from,
                                              double progress);

// Accumulates the decomposed components |to| with |from| using the
// routines described in
// https://www.w3.org/TR/css-transforms-2/#combining-transform-lists
CRUI_EXPORT
DecomposedTransform AccumulateDecomposedTransforms(
    const DecomposedTransform& to,
    const DecomposedTransform& from);

// Calculates a transform with a transformed origin. The resulting transform is
// created by composing P * T * P^-1 where P is a constant transform to the new
// origin.
CRUI_EXPORT
Transform TransformAboutPivot(const PointF& pivot, const Transform& transform);

// Calculates a transform which would transform |src| to |dst|.
CRUI_EXPORT
Transform TransformBetweenRects(const RectF& src, const RectF& dst);

// Returns the 2d axis transform that maps the clipping frustum to the square
// from [-1, -1] (the original bottom-left corner) to [1, 1] (the original
// top-right corner).
CRUI_EXPORT
AxisTransform2d OrthoProjectionTransform(float left,
                                         float right,
                                         float bottom,
                                         float top);

// Returns the 2d axis transform that maps from ([-1, -1] .. [1, 1]) to
// ([x, y] .. [x + width, y + height]).
CRUI_EXPORT
AxisTransform2d WindowTransform(int x, int y, int width, int height);

// Compute 2D scale if possible, clamped with ClampFloatGeometry().
CRUI_EXPORT
cr::Optional<Vector2dF> TryComputeTransform2dScaleComponents(
    const Transform& transform);

// Compute 2D scale, and fall back to fallback_value if not possible.
CRUI_EXPORT
Vector2dF ComputeTransform2dScaleComponents(const Transform& transform,
                                            float fallback_value);

// Returns an approximate max scale value of the transform even if it has
// perspective. Prefer to use ComputeTransform2dScaleComponents if there is no
// perspective, since it can produce more accurate results.
CRUI_EXPORT
float ComputeApproximateMaxScale(const Transform& transform);

}  // namespace gfx
}  // namespace gfx

#endif  // UI_GFX_GEOMETRY_TRANSFORM_UTIL_H_