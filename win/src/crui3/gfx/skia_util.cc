// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/gfx/skia_util.h"

#include <stddef.h>
#include <stdint.h>

#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/skia/include/core/SkColorFilter.h"
#include "third_party/skia/include/core/SkColorPriv.h"
#include "third_party/skia/include/core/SkUnPreMultiply.h"
#include "third_party/skia/include/effects/SkBlurMaskFilter.h"
#include "third_party/skia/include/effects/SkGradientShader.h"
#include "third_party/skia/include/effects/SkLayerDrawLooper.h"
#include "crui/gfx/geometry/quad_f.h"
#include "crui/gfx/geometry/rect.h"
#include "crui/gfx/geometry/rect_f.h"
#include "crui/gfx/geometry/transform.h"
///#include "crui/gfx/image/image_skia_rep.h"
///#include "crui/gfx/shadow_value.h"

namespace crui {
namespace gfx {

SkPoint PointToSkPoint(const Point& point) {
  return SkPoint::Make(SkIntToScalar(point.x()), SkIntToScalar(point.y()));
}

SkIPoint PointToSkIPoint(const Point& point) {
  return SkIPoint::Make(point.x(), point.y());
}

SkPoint PointFToSkPoint(const PointF& point) {
  return SkPoint::Make(SkFloatToScalar(point.x()), SkFloatToScalar(point.y()));
}

SkRect RectToSkRect(const Rect& rect) {
  return SkRect::MakeXYWH(
      SkIntToScalar(rect.x()), SkIntToScalar(rect.y()),
      SkIntToScalar(rect.width()), SkIntToScalar(rect.height()));
}

SkIRect RectToSkIRect(const Rect& rect) {
  return SkIRect::MakeXYWH(rect.x(), rect.y(), rect.width(), rect.height());
}

Rect SkIRectToRect(const SkIRect& rect) {
  return Rect(rect.x(), rect.y(), rect.width(), rect.height());
}

SkRect RectFToSkRect(const RectF& rect) {
  return SkRect::MakeXYWH(SkFloatToScalar(rect.x()),
                          SkFloatToScalar(rect.y()),
                          SkFloatToScalar(rect.width()),
                          SkFloatToScalar(rect.height()));
}

RectF SkRectToRectF(const SkRect& rect) {
  return RectF(SkScalarToFloat(rect.x()),
               SkScalarToFloat(rect.y()),
               SkScalarToFloat(rect.width()),
               SkScalarToFloat(rect.height()));
}

SkSize SizeFToSkSize(const SizeF& size) {
  return SkSize::Make(SkFloatToScalar(size.width()),
                      SkFloatToScalar(size.height()));
}

SizeF SkSizeToSizeF(const SkSize& size) {
  return SizeF(SkScalarToFloat(size.width()), SkScalarToFloat(size.height()));
}

Size SkISizeToSize(const SkISize& size) {
  return Size(size.width(), size.height());
}

void TransformToFlattenedSkMatrix(const gfx::Transform& matrix,
                                  SkMatrix* flattened) {
  // Convert from 4x4 to 3x3 by dropping row 2 (counted from 0) and column 2.
  flattened->setAll(SkDoubleToScalar(matrix.rc(0, 0)), 
                    SkDoubleToScalar(matrix.rc(0, 1)), 
                    SkDoubleToScalar(matrix.rc(0, 3)),
                    SkDoubleToScalar(matrix.rc(1, 0)), 
                    SkDoubleToScalar(matrix.rc(1, 1)), 
                    SkDoubleToScalar(matrix.rc(1, 3)),
                    SkDoubleToScalar(matrix.rc(3, 0)), 
                    SkDoubleToScalar(matrix.rc(3, 1)), 
                    SkDoubleToScalar(matrix.rc(3, 3)));
}

///skia::RefPtr<SkShader> CreateImageRepShader(const gfx::ImageSkiaRep& image_rep,
///                                            SkShader::TileMode tile_mode,
///                                            const SkMatrix& local_matrix) {
///  return CreateImageRepShaderForScale(image_rep, tile_mode, local_matrix,
///                                      image_rep.scale());
///}

///skia::RefPtr<SkShader> CreateImageRepShaderForScale(
///    const gfx::ImageSkiaRep& image_rep,
///    SkShader::TileMode tile_mode,
///    const SkMatrix& local_matrix,
///    SkScalar scale) {
///  // Unscale matrix by |scale| such that the bitmap is drawn at the
///  // correct density.
///  // Convert skew and translation to pixel coordinates.
///  // Thus, for |bitmap_scale| = 2:
///  //   x scale = 2, x translation = 1 DIP,
///  // should be converted to
///  //   x scale = 1, x translation = 2 pixels.
///  SkMatrix shader_scale = local_matrix;
///  shader_scale.preScale(scale, scale);
///  shader_scale.setScaleX(local_matrix.getScaleX() / scale);
///  shader_scale.setScaleY(local_matrix.getScaleY() / scale);
///
///  skia::RefPtr<SkShader> shader = skia::AdoptRef(SkShader::CreateBitmapShader(
///      image_rep.sk_bitmap(), tile_mode, tile_mode, &shader_scale));
///  return shader;
///}

skia::RefPtr<SkShader> CreateGradientShader(int start_point,
                                            int end_point,
                                            SkColor start_color,
                                            SkColor end_color) {
  SkColor grad_colors[2] = { start_color, end_color};
  SkPoint grad_points[2];
  grad_points[0].iset(0, start_point);
  grad_points[1].iset(0, end_point);

  return skia::AdoptRef(SkGradientShader::CreateLinear(
      grad_points, grad_colors, NULL, 2, SkShader::kRepeat_TileMode));
}

static SkScalar RadiusToSigma(double radius) {
  // This captures historically what skia did under the hood. Now skia accepts
  // sigma, not radius, so we perform the conversion.
  return radius > 0 ? SkDoubleToScalar(0.57735f * radius + 0.5) : 0;
}

///skia::RefPtr<SkDrawLooper> CreateShadowDrawLooper(
///    const std::vector<ShadowValue>& shadows) {
///  if (shadows.empty())
///    return skia::RefPtr<SkDrawLooper>();
///
///  SkLayerDrawLooper::Builder looper_builder;
///
///  looper_builder.addLayer();  // top layer of the original.
///
///  SkLayerDrawLooper::LayerInfo layer_info;
///  layer_info.fPaintBits |= SkLayerDrawLooper::kMaskFilter_Bit;
///  layer_info.fPaintBits |= SkLayerDrawLooper::kColorFilter_Bit;
///  layer_info.fColorMode = SkXfermode::kSrc_Mode;
///
///  for (size_t i = 0; i < shadows.size(); ++i) {
///    const ShadowValue& shadow = shadows[i];
///
///    layer_info.fOffset.set(SkIntToScalar(shadow.x()),
///                           SkIntToScalar(shadow.y()));
///
///    // SkBlurMaskFilter's blur radius defines the range to extend the blur from
///    // original mask, which is half of blur amount as defined in ShadowValue.
///    skia::RefPtr<SkMaskFilter> blur_mask = skia::AdoptRef(
///        SkBlurMaskFilter::Create(kNormal_SkBlurStyle,
///                                 RadiusToSigma(shadow.blur() / 2),
///                                 SkBlurMaskFilter::kHighQuality_BlurFlag));
///    skia::RefPtr<SkColorFilter> color_filter = skia::AdoptRef(
///        SkColorFilter::CreateModeFilter(shadow.color(),
///                                        SkXfermode::kSrcIn_Mode));
///
///    SkPaint* paint = looper_builder.addLayer(layer_info);
///    paint->setMaskFilter(blur_mask.get());
///    paint->setColorFilter(color_filter.get());
///  }
///
///  return skia::AdoptRef<SkDrawLooper>(looper_builder.detachLooper());
///}

bool BitmapsAreEqual(const SkBitmap& bitmap1, const SkBitmap& bitmap2) {
  void* addr1 = NULL;
  void* addr2 = NULL;
  size_t size1 = 0;
  size_t size2 = 0;

  bitmap1.lockPixels();
  addr1 = bitmap1.getAddr32(0, 0);
  size1 = bitmap1.getSize();
  bitmap1.unlockPixels();

  bitmap2.lockPixels();
  addr2 = bitmap2.getAddr32(0, 0);
  size2 = bitmap2.getSize();
  bitmap2.unlockPixels();

  return (size1 == size2) && (0 == memcmp(addr1, addr2, bitmap1.getSize()));
}

void ConvertSkiaToRGBA(const unsigned char* skia,
                       int pixel_width,
                       unsigned char* rgba) {
  int total_length = pixel_width * 4;
  for (int i = 0; i < total_length; i += 4) {
    const uint32_t pixel_in = *reinterpret_cast<const uint32_t*>(&skia[i]);

    // Pack the components here.
    SkAlpha alpha = SkGetPackedA32(pixel_in);
    if (alpha != 0 && alpha != 255) {
      SkColor unmultiplied = SkUnPreMultiply::PMColorToColor(pixel_in);
      rgba[i + 0] = SkColorGetR(unmultiplied);
      rgba[i + 1] = SkColorGetG(unmultiplied);
      rgba[i + 2] = SkColorGetB(unmultiplied);
      rgba[i + 3] = alpha;
    } else {
      rgba[i + 0] = SkGetPackedR32(pixel_in);
      rgba[i + 1] = SkGetPackedG32(pixel_in);
      rgba[i + 2] = SkGetPackedB32(pixel_in);
      rgba[i + 3] = alpha;
    }
  }
}

void QuadFToSkPoints(const gfx::QuadF& quad, SkPoint points[4]) {
  points[0] = PointFToSkPoint(quad.p1());
  points[1] = PointFToSkPoint(quad.p2());
  points[2] = PointFToSkPoint(quad.p3());
  points[3] = PointFToSkPoint(quad.p4());
}

}  // namespace gfx
}  // namespace crui
