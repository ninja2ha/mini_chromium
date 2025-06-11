// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_TRANSFORM_UTIL_H_
#define UI_GFX_TRANSFORM_UTIL_H_

#include "crui/gfx/geometry/point.h"
#include "crui/gfx/geometry/quaternion.h"
#include "crui/gfx/geometry/transform.h"
#include "crui/base/ui_export.h"

namespace crui {
namespace gfx {

class Rect;
class RectF;

// Returns a scale transform at |anchor| point.
CRUI_EXPORT Transform GetScaleTransform(const Point& anchor,
                                        float scale);

// Contains the components of a factored transform. These components may be
// blended and recomposed.
struct CRUI_EXPORT DecomposedTransform {
  // The default constructor initializes the components in such a way that
  // if used with Compose below, will produce the identity transform.
  DecomposedTransform();

  SkScalar translate[3];
  SkScalar scale[3];
  SkScalar skew[3];
  SkScalar perspective[4];
  Quaternion quaternion;

  std::string ToString() const;

  // Copy and assign are allowed.
};

// Interpolates the decomposed components |to| with |from| using the
// routines described in http://www.w3.org/TR/css3-3d-transform/.
// |progress| is in the range [0, 1]. If 0 we will return |from|, if 1, we will
// return |to|.
CRUI_EXPORT DecomposedTransform
BlendDecomposedTransforms(const DecomposedTransform& to,
                          const DecomposedTransform& from,
                          double progress);

// Decomposes this transform into its translation, scale, skew, perspective,
// and rotation components following the routines detailed in this spec:
// http://www.w3.org/TR/css3-3d-transforms/.
CRUI_EXPORT bool DecomposeTransform(DecomposedTransform* out,
                                    const Transform& transform);

// Composes a transform from the given translation, scale, skew, prespective,
// and rotation components following the routines detailed in this spec:
// http://www.w3.org/TR/css3-3d-transforms/.
CRUI_EXPORT Transform
ComposeTransform(const DecomposedTransform& decomp);

CRUI_EXPORT bool SnapTransform(Transform* out,
                               const Transform& transform,
                               const Rect& viewport);

// Calculates a transform with a transformed origin. The resulting tranform is
// created by composing P * T * P^-1 where P is a constant transform to the new
// origin.
CRUI_EXPORT Transform TransformAboutPivot(const Point& pivot,
                                          const Transform& transform);

// Calculates a transform which would transform |src| to |dst|.
CRUI_EXPORT Transform TransformBetweenRects(const RectF& src,
                                            const RectF& dst);

}  // namespace gfx
}  // namespace crui

#endif  // UI_GFX_TRANSFORM_UTIL_H_