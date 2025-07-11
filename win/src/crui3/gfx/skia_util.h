// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_SKIA_UTIL_H_
#define UI_GFX_SKIA_UTIL_H_

#include <string>
#include <vector>

#include "crui/skia/ext/refptr.h"
#include "third_party/skia/include/core/SkColor.h"
#include "third_party/skia/include/core/SkRect.h"
#include "third_party/skia/include/core/SkShader.h"
#include "crui/gfx/geometry/quad_f.h"
#include "crui/gfx/geometry/size.h"
#include "crui/gfx/geometry/size_f.h"
#include "crui/base/ui_export.h"

class SkBitmap;
class SkDrawLooper;

namespace crui {
namespace gfx {

///class ImageSkiaRep;
class Point;
class PointF;
class Rect;
class RectF;
///class ShadowValue;
class Transform;

// Convert between Skia and gfx types.
CRUI_EXPORT SkPoint PointToSkPoint(const Point& point);
CRUI_EXPORT SkIPoint PointToSkIPoint(const Point& point);
CRUI_EXPORT SkPoint PointFToSkPoint(const PointF& point);
CRUI_EXPORT SkRect RectToSkRect(const Rect& rect);
CRUI_EXPORT SkIRect RectToSkIRect(const Rect& rect);
CRUI_EXPORT Rect SkIRectToRect(const SkIRect& rect);
CRUI_EXPORT SkRect RectFToSkRect(const RectF& rect);
CRUI_EXPORT RectF SkRectToRectF(const SkRect& rect);
CRUI_EXPORT SkSize SizeFToSkSize(const SizeF& size);
CRUI_EXPORT SizeF SkSizeToSizeF(const SkSize& size);
CRUI_EXPORT Size SkISizeToSize(const SkISize& size);

CRUI_EXPORT void QuadFToSkPoints(const gfx::QuadF& quad, SkPoint points[4]);

CRUI_EXPORT void TransformToFlattenedSkMatrix(const gfx::Transform& transform,
                                              SkMatrix* flattened);

// Creates a bitmap shader for the image rep with the image rep's scale factor.
// Sets the created shader's local matrix such that it displays the image rep at
// the correct scale factor.
// The shader's local matrix should not be changed after the shader is created.
// TODO(pkotwicz): Allow shader's local matrix to be changed after the shader
// is created.
//
///CRUI_EXPORT skia::RefPtr<SkShader> CreateImageRepShader(
///    const gfx::ImageSkiaRep& image_rep,
///    SkShader::TileMode tile_mode,
///    const SkMatrix& local_matrix);
///
///// Creates a bitmap shader for the image rep with the passed in scale factor.
///CRUI_EXPORT skia::RefPtr<SkShader> CreateImageRepShaderForScale(
///    const gfx::ImageSkiaRep& image_rep,
///    SkShader::TileMode tile_mode,
///    const SkMatrix& local_matrix,
///    SkScalar scale);

// Creates a vertical gradient shader. The caller owns the shader.
// Example usage to avoid leaks:
CRUI_EXPORT skia::RefPtr<SkShader> CreateGradientShader(int start_point,
                                                        int end_point,
                                                        SkColor start_color,
                                                        SkColor end_color);

// Creates a draw looper to generate |shadows|. The caller owns the draw looper.
// NULL is returned if |shadows| is empty since no draw looper is needed in
// this case.
///CRUI_EXPORT skia::RefPtr<SkDrawLooper> CreateShadowDrawLooper(
///    const std::vector<ShadowValue>& shadows);

// Returns true if the two bitmaps contain the same pixels.
CRUI_EXPORT bool BitmapsAreEqual(const SkBitmap& bitmap1,
                                 const SkBitmap& bitmap2);

// Converts Skia ARGB format pixels in |skia| to RGBA.
CRUI_EXPORT void ConvertSkiaToRGBA(const unsigned char* skia,
                                   int pixel_width,
                                   unsigned char* rgba);

}  // namespace gfx
}  // namespace crui

#endif  // UI_GFX_SKIA_UTIL_H_
