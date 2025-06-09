/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/Sk1DPathEffect.h"
#include "include/effects/Sk2DPathEffect.h"
#include "include/effects/SkAlphaThresholdFilter.h"
#include "include/effects/SkArithmeticMode.h"
#include "include/effects/SkArcToPathEffect.h"
#include "src/utils/SkBitmapSourceDeserializer.h"
#include "include/effects/SkBlurDrawLooper.h"
#include "include/effects/SkBlurImageFilter.h"
#include "include/effects/SkBlurMaskFilter.h"
#include "include/effects/SkColorCubeFilter.h"
#include "include/effects/SkColorFilterImageFilter.h"
#include "include/effects/SkColorMatrixFilter.h"
#include "include/effects/SkComposeImageFilter.h"
#include "include/effects/SkCornerPathEffect.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkDiscretePathEffect.h"
#include "include/effects/SkDisplacementMapEffect.h"
#include "include/effects/SkDropShadowImageFilter.h"
#include "include/effects/SkEmbossMaskFilter.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageSource.h"
#include "include/effects/SkLayerDrawLooper.h"
#include "include/effects/SkLayerRasterizer.h"
#include "include/effects/SkLerpXfermode.h"
#include "include/effects/SkLightingImageFilter.h"
#include "src/core/SkLightingShader.h"
#include "include/effects/SkLumaColorFilter.h"
#include "include/effects/SkMagnifierImageFilter.h"
#include "include/effects/SkMatrixConvolutionImageFilter.h"
#include "include/effects/SkMergeImageFilter.h"
#include "include/effects/SkMorphologyImageFilter.h"
#include "include/effects/SkOffsetImageFilter.h"
#include "include/effects/SkPaintImageFilter.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "include/effects/SkPictureImageFilter.h"
#include "include/effects/SkPixelXorXfermode.h"
#include "include/effects/SkTableColorFilter.h"
#include "include/effects/SkTestImageFilters.h"
#include "include/effects/SkTileImageFilter.h"
#include "include/effects/SkXfermodeImageFilter.h"

/*
 *  None of these are strictly "required" for Skia to operate.
 *
 *  These are the bulk of our "effects" -- subclasses of various effects on SkPaint.
 *
 *  Clients should feel free to dup this file and modify it as needed. This function "InitEffects"
 *  will automatically be called before any of skia's effects are asked to be deserialized.
 */
void SkFlattenable::PrivateInitializer::InitEffects() {
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkBitmapSourceDeserializer)

    // MaskFilter
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkEmbossMaskFilter)
    SkBlurMaskFilter::InitializeFlattenables();

    // DrawLooper
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkBlurDrawLooper)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLayerDrawLooper)

    // Rasterizer
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLayerRasterizer)

    // ColorFilter
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkColorCubeFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkColorMatrixFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLumaColorFilter)
    SkAlphaThresholdFilter::InitializeFlattenables();
    SkArithmeticMode::InitializeFlattenables();
    SkTableColorFilter::InitializeFlattenables();

    // Shader
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPerlinNoiseShader)
    SkGradientShader::InitializeFlattenables();
    SkLightingShader::InitializeFlattenables();

    // Xfermode
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLerpXfermode)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPixelXorXfermode)

    // PathEffect
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkArcToPathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkCornerPathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDashPathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDiscretePathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPath1DPathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLine2DPathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPath2DPathEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkSumPathEffect)

    // ImageFilter
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkBlurImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDilateImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDisplacementMapEffect)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDropShadowImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkErodeImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkImageSource)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPaintImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkPictureImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkTileImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkXfermodeImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkMagnifierImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkMatrixConvolutionImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkOffsetImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkComposeImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkMergeImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkColorFilterImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDownSampleImageFilter)
    SkLightingImageFilter::InitializeFlattenables();
}
