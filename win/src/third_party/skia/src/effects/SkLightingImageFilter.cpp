/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkLightingImageFilter.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkDevice.h"
#include "include/core/SkPoint3.h"
#include "src/core/SkReadBuffer.h"
#include "include/core/SkTypes.h"
#include "include/core/SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrContext.h"
#include "include/gpu/GrDrawContext.h"
#include "include/gpu/GrFragmentProcessor.h"
#include "include/gpu/GrInvariantOutput.h"
#include "include/gpu/GrPaint.h"
#include "include/gpu/SkGr.h"
#include "src/gpu/effects/GrSingleTextureEffect.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"

class GrGLDiffuseLightingEffect;
class GrGLSpecularLightingEffect;

// For brevity
typedef GrGLSLProgramDataManager::UniformHandle UniformHandle;
#endif

namespace {

const SkScalar gOneThird = SkIntToScalar(1) / 3;
const SkScalar gTwoThirds = SkIntToScalar(2) / 3;
const SkScalar gOneHalf = 0.5f;
const SkScalar gOneQuarter = 0.25f;

#if SK_SUPPORT_GPU
void setUniformPoint3(const GrGLSLProgramDataManager& pdman, UniformHandle uni,
                      const SkPoint3& point) {
    GR_STATIC_ASSERT(sizeof(SkPoint3) == 3 * sizeof(float));
    pdman.set3fv(uni, 1, &point.fX);
}

void setUniformNormal3(const GrGLSLProgramDataManager& pdman, UniformHandle uni,
                       const SkPoint3& point) {
    setUniformPoint3(pdman, uni, point);
}
#endif

// Shift matrix components to the left, as we advance pixels to the right.
inline void shiftMatrixLeft(int m[9]) {
    m[0] = m[1];
    m[3] = m[4];
    m[6] = m[7];
    m[1] = m[2];
    m[4] = m[5];
    m[7] = m[8];
}

static inline void fast_normalize(SkPoint3* vector) {
    // add a tiny bit so we don't have to worry about divide-by-zero
    SkScalar magSq = vector->dot(*vector) + SK_ScalarNearlyZero;
    SkScalar scale = sk_float_rsqrt(magSq);
    vector->fX *= scale;
    vector->fY *= scale;
    vector->fZ *= scale;
}

class DiffuseLightingType {
public:
    DiffuseLightingType(SkScalar kd)
        : fKD(kd) {}
    SkPMColor light(const SkPoint3& normal, const SkPoint3& surfaceTolight,
                    const SkPoint3& lightColor) const {
        SkScalar colorScale = SkScalarMul(fKD, normal.dot(surfaceTolight));
        colorScale = SkScalarClampMax(colorScale, SK_Scalar1);
        SkPoint3 color = lightColor.makeScale(colorScale);
        return SkPackARGB32(255,
                            SkClampMax(SkScalarRoundToInt(color.fX), 255),
                            SkClampMax(SkScalarRoundToInt(color.fY), 255),
                            SkClampMax(SkScalarRoundToInt(color.fZ), 255));
    }
private:
    SkScalar fKD;
};

static SkScalar max_component(const SkPoint3& p) {
    return p.x() > p.y() ? (p.x() > p.z() ? p.x() : p.z()) : (p.y() > p.z() ? p.y() : p.z());
}

class SpecularLightingType {
public:
    SpecularLightingType(SkScalar ks, SkScalar shininess)
        : fKS(ks), fShininess(shininess) {}
    SkPMColor light(const SkPoint3& normal, const SkPoint3& surfaceTolight,
                    const SkPoint3& lightColor) const {
        SkPoint3 halfDir(surfaceTolight);
        halfDir.fZ += SK_Scalar1;        // eye position is always (0, 0, 1)
        fast_normalize(&halfDir);
        SkScalar colorScale = SkScalarMul(fKS,
            SkScalarPow(normal.dot(halfDir), fShininess));
        colorScale = SkScalarClampMax(colorScale, SK_Scalar1);
        SkPoint3 color = lightColor.makeScale(colorScale);
        return SkPackARGB32(SkClampMax(SkScalarRoundToInt(max_component(color)), 255),
                            SkClampMax(SkScalarRoundToInt(color.fX), 255),
                            SkClampMax(SkScalarRoundToInt(color.fY), 255),
                            SkClampMax(SkScalarRoundToInt(color.fZ), 255));
    }
private:
    SkScalar fKS;
    SkScalar fShininess;
};

inline SkScalar sobel(int a, int b, int c, int d, int e, int f, SkScalar scale) {
    return SkScalarMul(SkIntToScalar(-a + b - 2 * c + 2 * d -e + f), scale);
}

inline SkPoint3 pointToNormal(SkScalar x, SkScalar y, SkScalar surfaceScale) {
    SkPoint3 vector = SkPoint3::Make(SkScalarMul(-x, surfaceScale),
                                     SkScalarMul(-y, surfaceScale),
                                     SK_Scalar1);
    fast_normalize(&vector);
    return vector;
}

inline SkPoint3 topLeftNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(0, 0, m[4], m[5], m[7], m[8], gTwoThirds),
                         sobel(0, 0, m[4], m[7], m[5], m[8], gTwoThirds),
                         surfaceScale);
}

inline SkPoint3 topNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(   0,    0, m[3], m[5], m[6], m[8], gOneThird),
                         sobel(m[3], m[6], m[4], m[7], m[5], m[8], gOneHalf),
                         surfaceScale);
}

inline SkPoint3 topRightNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(   0,    0, m[3], m[4], m[6], m[7], gTwoThirds),
                         sobel(m[3], m[6], m[4], m[7],    0,    0, gTwoThirds),
                         surfaceScale);
}

inline SkPoint3 leftNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(m[1], m[2], m[4], m[5], m[7], m[8], gOneHalf),
                         sobel(   0,    0, m[1], m[7], m[2], m[8], gOneThird),
                         surfaceScale);
}


inline SkPoint3 interiorNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(m[0], m[2], m[3], m[5], m[6], m[8], gOneQuarter),
                         sobel(m[0], m[6], m[1], m[7], m[2], m[8], gOneQuarter),
                         surfaceScale);
}

inline SkPoint3 rightNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(m[0], m[1], m[3], m[4], m[6], m[7], gOneHalf),
                         sobel(m[0], m[6], m[1], m[7],    0,    0, gOneThird),
                         surfaceScale);
}

inline SkPoint3 bottomLeftNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(m[1], m[2], m[4], m[5],    0,    0, gTwoThirds),
                         sobel(   0,    0, m[1], m[4], m[2], m[5], gTwoThirds),
                         surfaceScale);
}

inline SkPoint3 bottomNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(m[0], m[2], m[3], m[5],    0,    0, gOneThird),
                         sobel(m[0], m[3], m[1], m[4], m[2], m[5], gOneHalf),
                         surfaceScale);
}

inline SkPoint3 bottomRightNormal(int m[9], SkScalar surfaceScale) {
    return pointToNormal(sobel(m[0], m[1], m[3], m[4], 0,  0, gTwoThirds),
                         sobel(m[0], m[3], m[1], m[4], 0,  0, gTwoThirds),
                         surfaceScale);
}

template <class LightingType, class LightType> void lightBitmap(const LightingType& lightingType,
                                                                const SkImageFilterLight* light,
                                                                const SkBitmap& src,
                                                                SkBitmap* dst,
                                                                SkScalar surfaceScale,
                                                                const SkIRect& bounds) {
    SkASSERT(dst->width() == bounds.width() && dst->height() == bounds.height());
    const LightType* l = static_cast<const LightType*>(light);
    int left = bounds.left(), right = bounds.right();
    int bottom = bounds.bottom();
    int y = bounds.top();
    SkPMColor* dptr = dst->getAddr32(0, 0);
    {
        int x = left;
        const SkPMColor* row1 = src.getAddr32(x, y);
        const SkPMColor* row2 = src.getAddr32(x, y + 1);
        int m[9];
        m[4] = SkGetPackedA32(*row1++);
        m[5] = SkGetPackedA32(*row1++);
        m[7] = SkGetPackedA32(*row2++);
        m[8] = SkGetPackedA32(*row2++);
        SkPoint3 surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
        *dptr++ = lightingType.light(topLeftNormal(m, surfaceScale), surfaceToLight,
                                     l->lightColor(surfaceToLight));
        for (++x; x < right - 1; ++x)
        {
            shiftMatrixLeft(m);
            m[5] = SkGetPackedA32(*row1++);
            m[8] = SkGetPackedA32(*row2++);
            surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
            *dptr++ = lightingType.light(topNormal(m, surfaceScale), surfaceToLight,
                                         l->lightColor(surfaceToLight));
        }
        shiftMatrixLeft(m);
        surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
        *dptr++ = lightingType.light(topRightNormal(m, surfaceScale), surfaceToLight,
                                     l->lightColor(surfaceToLight));
    }

    for (++y; y < bottom - 1; ++y) {
        int x = left;
        const SkPMColor* row0 = src.getAddr32(x, y - 1);
        const SkPMColor* row1 = src.getAddr32(x, y);
        const SkPMColor* row2 = src.getAddr32(x, y + 1);
        int m[9];
        m[1] = SkGetPackedA32(*row0++);
        m[2] = SkGetPackedA32(*row0++);
        m[4] = SkGetPackedA32(*row1++);
        m[5] = SkGetPackedA32(*row1++);
        m[7] = SkGetPackedA32(*row2++);
        m[8] = SkGetPackedA32(*row2++);
        SkPoint3 surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
        *dptr++ = lightingType.light(leftNormal(m, surfaceScale), surfaceToLight,
                                     l->lightColor(surfaceToLight));
        for (++x; x < right - 1; ++x) {
            shiftMatrixLeft(m);
            m[2] = SkGetPackedA32(*row0++);
            m[5] = SkGetPackedA32(*row1++);
            m[8] = SkGetPackedA32(*row2++);
            surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
            *dptr++ = lightingType.light(interiorNormal(m, surfaceScale), surfaceToLight,
                                         l->lightColor(surfaceToLight));
        }
        shiftMatrixLeft(m);
        surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
        *dptr++ = lightingType.light(rightNormal(m, surfaceScale), surfaceToLight,
                                     l->lightColor(surfaceToLight));
    }

    {
        int x = left;
        const SkPMColor* row0 = src.getAddr32(x, bottom - 2);
        const SkPMColor* row1 = src.getAddr32(x, bottom - 1);
        int m[9];
        m[1] = SkGetPackedA32(*row0++);
        m[2] = SkGetPackedA32(*row0++);
        m[4] = SkGetPackedA32(*row1++);
        m[5] = SkGetPackedA32(*row1++);
        SkPoint3 surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
        *dptr++ = lightingType.light(bottomLeftNormal(m, surfaceScale), surfaceToLight,
                                     l->lightColor(surfaceToLight));
        for (++x; x < right - 1; ++x)
        {
            shiftMatrixLeft(m);
            m[2] = SkGetPackedA32(*row0++);
            m[5] = SkGetPackedA32(*row1++);
            surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
            *dptr++ = lightingType.light(bottomNormal(m, surfaceScale), surfaceToLight,
                                         l->lightColor(surfaceToLight));
        }
        shiftMatrixLeft(m);
        surfaceToLight = l->surfaceToLight(x, y, m[4], surfaceScale);
        *dptr++ = lightingType.light(bottomRightNormal(m, surfaceScale), surfaceToLight,
                                     l->lightColor(surfaceToLight));
    }
}

SkPoint3 readPoint3(SkReadBuffer& buffer) {
    SkPoint3 point;
    point.fX = buffer.readScalar();
    point.fY = buffer.readScalar();
    point.fZ = buffer.readScalar();
    buffer.validate(SkScalarIsFinite(point.fX) &&
                    SkScalarIsFinite(point.fY) &&
                    SkScalarIsFinite(point.fZ));
    return point;
};

void writePoint3(const SkPoint3& point, SkWriteBuffer& buffer) {
    buffer.writeScalar(point.fX);
    buffer.writeScalar(point.fY);
    buffer.writeScalar(point.fZ);
};

enum BoundaryMode {
    kTopLeft_BoundaryMode,
    kTop_BoundaryMode,
    kTopRight_BoundaryMode,
    kLeft_BoundaryMode,
    kInterior_BoundaryMode,
    kRight_BoundaryMode,
    kBottomLeft_BoundaryMode,
    kBottom_BoundaryMode,
    kBottomRight_BoundaryMode,

    kBoundaryModeCount,
};

class SkLightingImageFilterInternal : public SkLightingImageFilter {
protected:
    SkLightingImageFilterInternal(SkImageFilterLight* light,
                                  SkScalar surfaceScale,
                                  SkImageFilter* input,
                                  const CropRect* cropRect)
      : INHERITED(light, surfaceScale, input, cropRect) {}

#if SK_SUPPORT_GPU
    bool canFilterImageGPU() const override { return true; }
    bool filterImageGPU(Proxy*, const SkBitmap& src, const Context&,
                        SkBitmap* result, SkIPoint* offset) const override;
    virtual GrFragmentProcessor* getFragmentProcessor(GrTexture*,
                                                      const SkMatrix&,
                                                      const SkIRect& bounds,
                                                      BoundaryMode boundaryMode) const = 0;
#endif
private:
#if SK_SUPPORT_GPU
    void drawRect(GrDrawContext* drawContext,
                  GrTexture* src,
                  const SkMatrix& matrix,
                  const GrClip& clip,
                  const SkRect& dstRect,
                  BoundaryMode boundaryMode,
                  const SkIRect& bounds) const;
#endif
    typedef SkLightingImageFilter INHERITED;
};

#if SK_SUPPORT_GPU
void SkLightingImageFilterInternal::drawRect(GrDrawContext* drawContext,
                                             GrTexture* src,
                                             const SkMatrix& matrix,
                                             const GrClip& clip,
                                             const SkRect& dstRect,
                                             BoundaryMode boundaryMode,
                                             const SkIRect& bounds) const {
    SkRect srcRect = dstRect.makeOffset(SkIntToScalar(bounds.x()), SkIntToScalar(bounds.y()));
    GrPaint paint;
    GrFragmentProcessor* fp = this->getFragmentProcessor(src, matrix, bounds, boundaryMode);
    paint.addColorFragmentProcessor(fp)->unref();
    paint.setPorterDuffXPFactory(SkXfermode::kSrc_Mode);
    drawContext->fillRectToRect(clip, paint, SkMatrix::I(), dstRect, srcRect);
}

bool SkLightingImageFilterInternal::filterImageGPU(Proxy* proxy,
                                                   const SkBitmap& src,
                                                   const Context& ctx,
                                                   SkBitmap* result,
                                                   SkIPoint* offset) const {
    SkBitmap input = src;
    SkIPoint srcOffset = SkIPoint::Make(0, 0);
    if (!this->filterInputGPU(0, proxy, src, ctx, &input, &srcOffset)) {
        return false;
    }
    SkIRect bounds;
    if (!this->applyCropRect(ctx, proxy, input, &srcOffset, &bounds, &input)) {
        return false;
    }
    SkRect dstRect = SkRect::MakeWH(SkIntToScalar(bounds.width()),
                                    SkIntToScalar(bounds.height()));
    GrTexture* srcTexture = input.getTexture();
    GrContext* context = srcTexture->getContext();

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag,
    desc.fWidth = bounds.width();
    desc.fHeight = bounds.height();
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    SkAutoTUnref<GrTexture> dst(context->textureProvider()->createApproxTexture(desc));
    if (!dst) {
        return false;
    }

    // setup new clip
    GrClip clip(dstRect);

    offset->fX = bounds.left();
    offset->fY = bounds.top();
    SkMatrix matrix(ctx.ctm());
    matrix.postTranslate(SkIntToScalar(-bounds.left()), SkIntToScalar(-bounds.top()));
    bounds.offset(-srcOffset);
    SkRect topLeft = SkRect::MakeXYWH(0, 0, 1, 1);
    SkRect top = SkRect::MakeXYWH(1, 0, dstRect.width() - 2, 1);
    SkRect topRight = SkRect::MakeXYWH(dstRect.width() - 1, 0, 1, 1);
    SkRect left = SkRect::MakeXYWH(0, 1, 1, dstRect.height() - 2);
    SkRect interior = dstRect.makeInset(1, 1);
    SkRect right = SkRect::MakeXYWH(dstRect.width() - 1, 1, 1, dstRect.height() - 2);
    SkRect bottomLeft = SkRect::MakeXYWH(0, dstRect.height() - 1, 1, 1);
    SkRect bottom = SkRect::MakeXYWH(1, dstRect.height() - 1, dstRect.width() - 2, 1);
    SkRect bottomRight = SkRect::MakeXYWH(dstRect.width() - 1, dstRect.height() - 1, 1, 1);

    SkAutoTUnref<GrDrawContext> drawContext(context->drawContext(dst->asRenderTarget()));
    if (!drawContext) {
        return false;
    }

    this->drawRect(drawContext, srcTexture, matrix, clip, topLeft, kTopLeft_BoundaryMode, bounds);
    this->drawRect(drawContext, srcTexture, matrix, clip, top, kTop_BoundaryMode, bounds);
    this->drawRect(drawContext, srcTexture, matrix, clip, topRight, kTopRight_BoundaryMode,
                   bounds);
    this->drawRect(drawContext, srcTexture, matrix, clip, left, kLeft_BoundaryMode, bounds);
    this->drawRect(drawContext, srcTexture, matrix, clip, interior, kInterior_BoundaryMode,
                   bounds);
    this->drawRect(drawContext, srcTexture, matrix, clip, right, kRight_BoundaryMode, bounds);
    this->drawRect(drawContext, srcTexture, matrix, clip, bottomLeft, kBottomLeft_BoundaryMode,
                   bounds);
    this->drawRect(drawContext, srcTexture, matrix, clip, bottom, kBottom_BoundaryMode, bounds);
    this->drawRect(drawContext, srcTexture, matrix, clip, bottomRight,
                   kBottomRight_BoundaryMode, bounds);
    GrWrapTextureInBitmap(dst, bounds.width(), bounds.height(), false, result);
    return true;
}
#endif

class SkDiffuseLightingImageFilter : public SkLightingImageFilterInternal {
public:
    static SkImageFilter* Create(SkImageFilterLight* light, SkScalar surfaceScale,
                                 SkScalar kd, SkImageFilter*,
                                 const CropRect*);

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDiffuseLightingImageFilter)
    SkScalar kd() const { return fKD; }

protected:
    SkDiffuseLightingImageFilter(SkImageFilterLight* light, SkScalar surfaceScale,
                                 SkScalar kd, SkImageFilter* input, const CropRect* cropRect);
    void flatten(SkWriteBuffer& buffer) const override;
    bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                       SkBitmap* result, SkIPoint* offset) const override;
#if SK_SUPPORT_GPU
    GrFragmentProcessor* getFragmentProcessor(GrTexture*, const SkMatrix&, const SkIRect& bounds,
                                              BoundaryMode) const override;
#endif

private:
    friend class SkLightingImageFilter;
    typedef SkLightingImageFilterInternal INHERITED;
    SkScalar fKD;
};

class SkSpecularLightingImageFilter : public SkLightingImageFilterInternal {
public:
    static SkImageFilter* Create(SkImageFilterLight* light, SkScalar surfaceScale,
                                 SkScalar ks, SkScalar shininess, SkImageFilter*, const CropRect*);

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkSpecularLightingImageFilter)

    SkScalar ks() const { return fKS; }
    SkScalar shininess() const { return fShininess; }

protected:
    SkSpecularLightingImageFilter(SkImageFilterLight* light, SkScalar surfaceScale, SkScalar ks,
                                  SkScalar shininess, SkImageFilter* input, const CropRect*);
    void flatten(SkWriteBuffer& buffer) const override;
    bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                       SkBitmap* result, SkIPoint* offset) const override;
#if SK_SUPPORT_GPU
    GrFragmentProcessor* getFragmentProcessor(GrTexture*, const SkMatrix&, const SkIRect& bounds,
                                              BoundaryMode) const override;
#endif

private:
    SkScalar fKS;
    SkScalar fShininess;
    friend class SkLightingImageFilter;
    typedef SkLightingImageFilterInternal INHERITED;
};

#if SK_SUPPORT_GPU

class GrLightingEffect : public GrSingleTextureEffect {
public:
    GrLightingEffect(GrTexture* texture, const SkImageFilterLight* light, SkScalar surfaceScale,
                     const SkMatrix& matrix, BoundaryMode boundaryMode);
    ~GrLightingEffect() override;

    const SkImageFilterLight* light() const { return fLight; }
    SkScalar surfaceScale() const { return fSurfaceScale; }
    const SkMatrix& filterMatrix() const { return fFilterMatrix; }
    BoundaryMode boundaryMode() const { return fBoundaryMode; }

protected:
    bool onIsEqual(const GrFragmentProcessor&) const override;

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        // lighting shaders are complicated. We just throw up our hands.
        inout->mulByUnknownFourComponents();
    }

private:
    const SkImageFilterLight* fLight;
    SkScalar fSurfaceScale;
    SkMatrix fFilterMatrix;
    BoundaryMode fBoundaryMode;

    typedef GrSingleTextureEffect INHERITED;
};

class GrDiffuseLightingEffect : public GrLightingEffect {
public:
    static GrFragmentProcessor* Create(GrTexture* texture,
                                       const SkImageFilterLight* light,
                                       SkScalar surfaceScale,
                                       const SkMatrix& matrix,
                                       SkScalar kd,
                                       BoundaryMode boundaryMode) {
        return new GrDiffuseLightingEffect(texture, light, surfaceScale, matrix, kd, boundaryMode);
    }

    const char* name() const override { return "DiffuseLighting"; }

    SkScalar kd() const { return fKD; }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    GrDiffuseLightingEffect(GrTexture* texture,
                            const SkImageFilterLight* light,
                            SkScalar surfaceScale,
                            const SkMatrix& matrix,
                            SkScalar kd,
                            BoundaryMode boundaryMode);

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;
    typedef GrLightingEffect INHERITED;
    SkScalar fKD;
};

class GrSpecularLightingEffect : public GrLightingEffect {
public:
    static GrFragmentProcessor* Create(GrTexture* texture,
                                       const SkImageFilterLight* light,
                                       SkScalar surfaceScale,
                                       const SkMatrix& matrix,
                                       SkScalar ks,
                                       SkScalar shininess,
                                       BoundaryMode boundaryMode) {
        return new GrSpecularLightingEffect(texture, light, surfaceScale, matrix, ks, shininess,
                                            boundaryMode);
    }

    const char* name() const override { return "SpecularLighting"; }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    SkScalar ks() const { return fKS; }
    SkScalar shininess() const { return fShininess; }

private:
    void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    GrSpecularLightingEffect(GrTexture* texture,
                             const SkImageFilterLight* light,
                             SkScalar surfaceScale,
                             const SkMatrix& matrix,
                             SkScalar ks,
                             SkScalar shininess,
                             BoundaryMode boundaryMode);

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;
    typedef GrLightingEffect INHERITED;
    SkScalar fKS;
    SkScalar fShininess;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLLight {
public:
    virtual ~GrGLLight() {}

    /**
     * This is called by GrGLLightingEffect::emitCode() before either of the two virtual functions
     * below. It adds a vec3f uniform visible in the FS that represents the constant light color.
     */
    void emitLightColorUniform(GrGLSLUniformHandler*);

    /**
     * These two functions are called from GrGLLightingEffect's emitCode() function.
     * emitSurfaceToLight places an expression in param out that is the vector from the surface to
     * the light. The expression will be used in the FS. emitLightColor writes an expression into
     * the FS that is the color of the light. Either function may add functions and/or uniforms to
     * the FS. The default of emitLightColor appends the name of the constant light color uniform
     * and so this function only needs to be overridden if the light color varies spatially.
     */
    virtual void emitSurfaceToLight(GrGLSLUniformHandler*,
                                    GrGLSLFragmentBuilder*,
                                    const char* z) = 0;
    virtual void emitLightColor(GrGLSLUniformHandler*,
                                GrGLSLFragmentBuilder*,
                                const char *surfaceToLight);

    // This is called from GrGLLightingEffect's setData(). Subclasses of GrGLLight must call
    // INHERITED::setData().
    virtual void setData(const GrGLSLProgramDataManager&, const SkImageFilterLight* light) const;

protected:
    /**
     * Gets the constant light color uniform. Subclasses can use this in their emitLightColor
     * function.
     */
    UniformHandle lightColorUni() const { return fColorUni; }

private:
    UniformHandle fColorUni;

    typedef SkRefCnt INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLDistantLight : public GrGLLight {
public:
    virtual ~GrGLDistantLight() {}
    void setData(const GrGLSLProgramDataManager&, const SkImageFilterLight* light) const override;
    void emitSurfaceToLight(GrGLSLUniformHandler*, GrGLSLFragmentBuilder*, const char* z) override;

private:
    typedef GrGLLight INHERITED;
    UniformHandle fDirectionUni;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLPointLight : public GrGLLight {
public:
    virtual ~GrGLPointLight() {}
    void setData(const GrGLSLProgramDataManager&, const SkImageFilterLight* light) const override;
    void emitSurfaceToLight(GrGLSLUniformHandler*, GrGLSLFragmentBuilder*, const char* z) override;

private:
    typedef GrGLLight INHERITED;
    UniformHandle fLocationUni;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLSpotLight : public GrGLLight {
public:
    virtual ~GrGLSpotLight() {}
    void setData(const GrGLSLProgramDataManager&, const SkImageFilterLight* light) const override;
    void emitSurfaceToLight(GrGLSLUniformHandler*, GrGLSLFragmentBuilder*, const char* z) override;
    void emitLightColor(GrGLSLUniformHandler*,
                        GrGLSLFragmentBuilder*,
                        const char *surfaceToLight) override;

private:
    typedef GrGLLight INHERITED;

    SkString        fLightColorFunc;
    UniformHandle   fLocationUni;
    UniformHandle   fExponentUni;
    UniformHandle   fCosOuterConeAngleUni;
    UniformHandle   fCosInnerConeAngleUni;
    UniformHandle   fConeScaleUni;
    UniformHandle   fSUni;
};
#else

class GrGLLight;

#endif

};

///////////////////////////////////////////////////////////////////////////////

class SkImageFilterLight : public SkRefCnt {
public:
    

    enum LightType {
        kDistant_LightType,
        kPoint_LightType,
        kSpot_LightType,
    };
    virtual LightType type() const = 0;
    const SkPoint3& color() const { return fColor; }
    virtual GrGLLight* createGLLight() const = 0;
    virtual bool isEqual(const SkImageFilterLight& other) const {
        return fColor == other.fColor;
    }
    // Called to know whether the generated GrGLLight will require access to the fragment position.
    virtual bool requiresFragmentPosition() const = 0;
    virtual SkImageFilterLight* transform(const SkMatrix& matrix) const = 0;

    // Defined below SkLight's subclasses.
    void flattenLight(SkWriteBuffer& buffer) const;
    static SkImageFilterLight* UnflattenLight(SkReadBuffer& buffer);

protected:
    SkImageFilterLight(SkColor color) {
        fColor = SkPoint3::Make(SkIntToScalar(SkColorGetR(color)),
                                SkIntToScalar(SkColorGetG(color)),
                                SkIntToScalar(SkColorGetB(color)));
    }
    SkImageFilterLight(const SkPoint3& color)
      : fColor(color) {}
    SkImageFilterLight(SkReadBuffer& buffer) {
        fColor = readPoint3(buffer);
    }

    virtual void onFlattenLight(SkWriteBuffer& buffer) const = 0;


private:
    typedef SkRefCnt INHERITED;
    SkPoint3 fColor;
};

///////////////////////////////////////////////////////////////////////////////

class SkDistantLight : public SkImageFilterLight {
public:
    SkDistantLight(const SkPoint3& direction, SkColor color)
      : INHERITED(color), fDirection(direction) {
    }

    SkPoint3 surfaceToLight(int x, int y, int z, SkScalar surfaceScale) const {
        return fDirection;
    };
    const SkPoint3& lightColor(const SkPoint3&) const { return this->color(); }
    LightType type() const override { return kDistant_LightType; }
    const SkPoint3& direction() const { return fDirection; }
    GrGLLight* createGLLight() const override {
#if SK_SUPPORT_GPU
        return new GrGLDistantLight;
#else
        SkDEBUGFAIL("Should not call in GPU-less build");
        return nullptr;
#endif
    }
    bool requiresFragmentPosition() const override { return false; }

    bool isEqual(const SkImageFilterLight& other) const override {
        if (other.type() != kDistant_LightType) {
            return false;
        }

        const SkDistantLight& o = static_cast<const SkDistantLight&>(other);
        return INHERITED::isEqual(other) &&
               fDirection == o.fDirection;
    }

    SkDistantLight(SkReadBuffer& buffer) : INHERITED(buffer) {
        fDirection = readPoint3(buffer);
    }

protected:
    SkDistantLight(const SkPoint3& direction, const SkPoint3& color)
      : INHERITED(color), fDirection(direction) {
    }
    SkImageFilterLight* transform(const SkMatrix& matrix) const override {
        return new SkDistantLight(direction(), color());
    }
    void onFlattenLight(SkWriteBuffer& buffer) const override {
        writePoint3(fDirection, buffer);
    }

private:
    SkPoint3 fDirection;

    typedef SkImageFilterLight INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class SkPointLight : public SkImageFilterLight {
public:
    SkPointLight(const SkPoint3& location, SkColor color)
     : INHERITED(color), fLocation(location) {}

    SkPoint3 surfaceToLight(int x, int y, int z, SkScalar surfaceScale) const {
        SkPoint3 direction = SkPoint3::Make(fLocation.fX - SkIntToScalar(x),
                                            fLocation.fY - SkIntToScalar(y),
                                            fLocation.fZ - SkScalarMul(SkIntToScalar(z),
                                                                       surfaceScale));
        fast_normalize(&direction);
        return direction;
    };
    const SkPoint3& lightColor(const SkPoint3&) const { return this->color(); }
    LightType type() const override { return kPoint_LightType; }
    const SkPoint3& location() const { return fLocation; }
    GrGLLight* createGLLight() const override {
#if SK_SUPPORT_GPU
        return new GrGLPointLight;
#else
        SkDEBUGFAIL("Should not call in GPU-less build");
        return nullptr;
#endif
    }
    bool requiresFragmentPosition() const override { return true; }
    bool isEqual(const SkImageFilterLight& other) const override {
        if (other.type() != kPoint_LightType) {
            return false;
        }
        const SkPointLight& o = static_cast<const SkPointLight&>(other);
        return INHERITED::isEqual(other) &&
               fLocation == o.fLocation;
    }
    SkImageFilterLight* transform(const SkMatrix& matrix) const override {
        SkPoint location2 = SkPoint::Make(fLocation.fX, fLocation.fY);
        matrix.mapPoints(&location2, 1);
        // Use X scale and Y scale on Z and average the result
        SkPoint locationZ = SkPoint::Make(fLocation.fZ, fLocation.fZ);
        matrix.mapVectors(&locationZ, 1);
        SkPoint3 location = SkPoint3::Make(location2.fX, 
                                           location2.fY, 
                                           SkScalarAve(locationZ.fX, locationZ.fY));
        return new SkPointLight(location, color());
    }

    SkPointLight(SkReadBuffer& buffer) : INHERITED(buffer) {
        fLocation = readPoint3(buffer);
    }

protected:
    SkPointLight(const SkPoint3& location, const SkPoint3& color)
     : INHERITED(color), fLocation(location) {}
    void onFlattenLight(SkWriteBuffer& buffer) const override {
        writePoint3(fLocation, buffer);
    }

private:
    SkPoint3 fLocation;

    typedef SkImageFilterLight INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class SkSpotLight : public SkImageFilterLight {
public:
    SkSpotLight(const SkPoint3& location,
                const SkPoint3& target,
                SkScalar specularExponent,
                SkScalar cutoffAngle,
                SkColor color)
     : INHERITED(color),
       fLocation(location),
       fTarget(target),
       fSpecularExponent(SkScalarPin(specularExponent, kSpecularExponentMin, kSpecularExponentMax))
    {
       fS = target - location;
       fast_normalize(&fS);
       fCosOuterConeAngle = SkScalarCos(SkDegreesToRadians(cutoffAngle));
       const SkScalar antiAliasThreshold = 0.016f;
       fCosInnerConeAngle = fCosOuterConeAngle + antiAliasThreshold;
       fConeScale = SkScalarInvert(antiAliasThreshold);
    }

    SkImageFilterLight* transform(const SkMatrix& matrix) const override {
        SkPoint location2 = SkPoint::Make(fLocation.fX, fLocation.fY);
        matrix.mapPoints(&location2, 1);
        // Use X scale and Y scale on Z and average the result
        SkPoint locationZ = SkPoint::Make(fLocation.fZ, fLocation.fZ);
        matrix.mapVectors(&locationZ, 1);
        SkPoint3 location = SkPoint3::Make(location2.fX, location2.fY,
                                           SkScalarAve(locationZ.fX, locationZ.fY));
        SkPoint target2 = SkPoint::Make(fTarget.fX, fTarget.fY);
        matrix.mapPoints(&target2, 1);
        SkPoint targetZ = SkPoint::Make(fTarget.fZ, fTarget.fZ);
        matrix.mapVectors(&targetZ, 1);
        SkPoint3 target = SkPoint3::Make(target2.fX, target2.fY,
                                         SkScalarAve(targetZ.fX, targetZ.fY));
        SkPoint3 s = target - location;
        fast_normalize(&s);
        return new SkSpotLight(location,
                               target,
                               fSpecularExponent,
                               fCosOuterConeAngle,
                               fCosInnerConeAngle,
                               fConeScale,
                               s,
                               color());
    }

    SkPoint3 surfaceToLight(int x, int y, int z, SkScalar surfaceScale) const {
        SkPoint3 direction = SkPoint3::Make(fLocation.fX - SkIntToScalar(x),
                                            fLocation.fY - SkIntToScalar(y),
                                            fLocation.fZ - SkScalarMul(SkIntToScalar(z),
                                                                       surfaceScale));
        fast_normalize(&direction);
        return direction;
    };
    SkPoint3 lightColor(const SkPoint3& surfaceToLight) const {
        SkScalar cosAngle = -surfaceToLight.dot(fS);
        SkScalar scale = 0;
        if (cosAngle >= fCosOuterConeAngle) {
            scale = SkScalarPow(cosAngle, fSpecularExponent);
            if (cosAngle < fCosInnerConeAngle) {
                scale = SkScalarMul(scale, cosAngle - fCosOuterConeAngle);
                scale *= fConeScale;
            }
        }
        return this->color().makeScale(scale);
    }
    GrGLLight* createGLLight() const override {
#if SK_SUPPORT_GPU
        return new GrGLSpotLight;
#else
        SkDEBUGFAIL("Should not call in GPU-less build");
        return nullptr;
#endif
    }
    bool requiresFragmentPosition() const override { return true; }
    LightType type() const override { return kSpot_LightType; }
    const SkPoint3& location() const { return fLocation; }
    const SkPoint3& target() const { return fTarget; }
    SkScalar specularExponent() const { return fSpecularExponent; }
    SkScalar cosInnerConeAngle() const { return fCosInnerConeAngle; }
    SkScalar cosOuterConeAngle() const { return fCosOuterConeAngle; }
    SkScalar coneScale() const { return fConeScale; }
    const SkPoint3& s() const { return fS; }

    SkSpotLight(SkReadBuffer& buffer) : INHERITED(buffer) {
        fLocation = readPoint3(buffer);
        fTarget = readPoint3(buffer);
        fSpecularExponent = buffer.readScalar();
        fCosOuterConeAngle = buffer.readScalar();
        fCosInnerConeAngle = buffer.readScalar();
        fConeScale = buffer.readScalar();
        fS = readPoint3(buffer);
        buffer.validate(SkScalarIsFinite(fSpecularExponent) &&
                        SkScalarIsFinite(fCosOuterConeAngle) &&
                        SkScalarIsFinite(fCosInnerConeAngle) &&
                        SkScalarIsFinite(fConeScale));
    }
protected:
    SkSpotLight(const SkPoint3& location,
                const SkPoint3& target,
                SkScalar specularExponent,
                SkScalar cosOuterConeAngle,
                SkScalar cosInnerConeAngle,
                SkScalar coneScale,
                const SkPoint3& s,
                const SkPoint3& color)
     : INHERITED(color),
       fLocation(location),
       fTarget(target),
       fSpecularExponent(specularExponent),
       fCosOuterConeAngle(cosOuterConeAngle),
       fCosInnerConeAngle(cosInnerConeAngle),
       fConeScale(coneScale),
       fS(s)
    {
    }
    void onFlattenLight(SkWriteBuffer& buffer) const override {
        writePoint3(fLocation, buffer);
        writePoint3(fTarget, buffer);
        buffer.writeScalar(fSpecularExponent);
        buffer.writeScalar(fCosOuterConeAngle);
        buffer.writeScalar(fCosInnerConeAngle);
        buffer.writeScalar(fConeScale);
        writePoint3(fS, buffer);
    }

    bool isEqual(const SkImageFilterLight& other) const override {
        if (other.type() != kSpot_LightType) {
            return false;
        }

        const SkSpotLight& o = static_cast<const SkSpotLight&>(other);
        return INHERITED::isEqual(other) &&
               fLocation == o.fLocation &&
               fTarget == o.fTarget &&
               fSpecularExponent == o.fSpecularExponent &&
               fCosOuterConeAngle == o.fCosOuterConeAngle;
    }

private:
    static const SkScalar kSpecularExponentMin;
    static const SkScalar kSpecularExponentMax;

    SkPoint3 fLocation;
    SkPoint3 fTarget;
    SkScalar fSpecularExponent;
    SkScalar fCosOuterConeAngle;
    SkScalar fCosInnerConeAngle;
    SkScalar fConeScale;
    SkPoint3 fS;

    typedef SkImageFilterLight INHERITED;
};

// According to the spec, the specular term should be in the range [1, 128] :
// http://www.w3.org/TR/SVG/filters.html#feSpecularLightingSpecularExponentAttribute
const SkScalar SkSpotLight::kSpecularExponentMin = 1.0f;
const SkScalar SkSpotLight::kSpecularExponentMax = 128.0f;

///////////////////////////////////////////////////////////////////////////////

void SkImageFilterLight::flattenLight(SkWriteBuffer& buffer) const {
    // Write type first, then baseclass, then subclass.
    buffer.writeInt(this->type());
    writePoint3(fColor, buffer);
    this->onFlattenLight(buffer);
}

/*static*/ SkImageFilterLight* SkImageFilterLight::UnflattenLight(SkReadBuffer& buffer) {
    // Read type first.
    const SkImageFilterLight::LightType type = (SkImageFilterLight::LightType)buffer.readInt();
    switch (type) {
        // Each of these constructors must first call SkLight's, so we'll read the baseclass
        // then subclass, same order as flattenLight.
        case SkImageFilterLight::kDistant_LightType:
            return new SkDistantLight(buffer);
        case SkImageFilterLight::kPoint_LightType:
            return new SkPointLight(buffer);
        case SkImageFilterLight::kSpot_LightType:
            return new SkSpotLight(buffer);
        default:
            SkDEBUGFAIL("Unknown LightType.");
            buffer.validate(false);
            return nullptr;
    }
}
///////////////////////////////////////////////////////////////////////////////

SkLightingImageFilter::SkLightingImageFilter(SkImageFilterLight* light, SkScalar surfaceScale,
                                             SkImageFilter* input, const CropRect* cropRect)
  : INHERITED(1, &input, cropRect)
  , fLight(SkRef(light))
  , fSurfaceScale(surfaceScale / 255)
{}

SkImageFilter* SkLightingImageFilter::CreateDistantLitDiffuse(const SkPoint3& direction,
                                                              SkColor lightColor,
                                                              SkScalar surfaceScale,
                                                              SkScalar kd,
                                                              SkImageFilter* input,
                                                              const CropRect* cropRect) {
    SkAutoTUnref<SkImageFilterLight> light(new SkDistantLight(direction, lightColor));
    return SkDiffuseLightingImageFilter::Create(light, surfaceScale, kd, input, cropRect);
}

SkImageFilter* SkLightingImageFilter::CreatePointLitDiffuse(const SkPoint3& location,
                                                            SkColor lightColor,
                                                            SkScalar surfaceScale,
                                                            SkScalar kd,
                                                            SkImageFilter* input,
                                                            const CropRect* cropRect) {
    SkAutoTUnref<SkImageFilterLight> light(new SkPointLight(location, lightColor));
    return SkDiffuseLightingImageFilter::Create(light, surfaceScale, kd, input, cropRect);
}

SkImageFilter* SkLightingImageFilter::CreateSpotLitDiffuse(const SkPoint3& location,
                                                           const SkPoint3& target,
                                                           SkScalar specularExponent,
                                                           SkScalar cutoffAngle,
                                                           SkColor lightColor,
                                                           SkScalar surfaceScale,
                                                           SkScalar kd,
                                                           SkImageFilter* input,
                                                           const CropRect* cropRect) {
    SkAutoTUnref<SkImageFilterLight> light(
            new SkSpotLight(location, target, specularExponent, cutoffAngle, lightColor));
    return SkDiffuseLightingImageFilter::Create(light, surfaceScale, kd, input, cropRect);
}

SkImageFilter* SkLightingImageFilter::CreateDistantLitSpecular(const SkPoint3& direction,
                                                               SkColor lightColor,
                                                               SkScalar surfaceScale,
                                                               SkScalar ks,
                                                               SkScalar shine,
                                                               SkImageFilter* input,
                                                               const CropRect* cropRect) {
    SkAutoTUnref<SkImageFilterLight> light(new SkDistantLight(direction, lightColor));
    return SkSpecularLightingImageFilter::Create(light, surfaceScale, ks, shine, input, cropRect);
}

SkImageFilter* SkLightingImageFilter::CreatePointLitSpecular(const SkPoint3& location,
                                                             SkColor lightColor,
                                                             SkScalar surfaceScale,
                                                             SkScalar ks,
                                                             SkScalar shine,
                                                             SkImageFilter* input,
                                                             const CropRect* cropRect) {
    SkAutoTUnref<SkImageFilterLight> light(new SkPointLight(location, lightColor));
    return SkSpecularLightingImageFilter::Create(light, surfaceScale, ks, shine, input, cropRect);
}

SkImageFilter* SkLightingImageFilter::CreateSpotLitSpecular(const SkPoint3& location,
                                                            const SkPoint3& target,
                                                            SkScalar specularExponent,
                                                            SkScalar cutoffAngle,
                                                            SkColor lightColor,
                                                            SkScalar surfaceScale,
                                                            SkScalar ks,
                                                            SkScalar shine,
                                                            SkImageFilter* input,
                                                            const CropRect* cropRect) {
    SkAutoTUnref<SkImageFilterLight> light(
            new SkSpotLight(location, target, specularExponent, cutoffAngle, lightColor));
    return SkSpecularLightingImageFilter::Create(light, surfaceScale, ks, shine, input, cropRect);
}

SkLightingImageFilter::~SkLightingImageFilter() {}

void SkLightingImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    fLight->flattenLight(buffer);
    buffer.writeScalar(fSurfaceScale * 255);
}

///////////////////////////////////////////////////////////////////////////////

SkImageFilter* SkDiffuseLightingImageFilter::Create(SkImageFilterLight* light,
                                                    SkScalar surfaceScale,
                                                    SkScalar kd,
                                                    SkImageFilter* input,
                                                    const CropRect* cropRect) {
    if (nullptr == light) {
        return nullptr;
    }
    if (!SkScalarIsFinite(surfaceScale) || !SkScalarIsFinite(kd)) {
        return nullptr;
    }
    // According to the spec, kd can be any non-negative number :
    // http://www.w3.org/TR/SVG/filters.html#feDiffuseLightingElement
    if (kd < 0) {
        return nullptr;
    }
    return new SkDiffuseLightingImageFilter(light, surfaceScale, kd, input, cropRect);
}

SkDiffuseLightingImageFilter::SkDiffuseLightingImageFilter(SkImageFilterLight* light,
                                                           SkScalar surfaceScale,
                                                           SkScalar kd,
                                                           SkImageFilter* input,
                                                           const CropRect* cropRect)
  : INHERITED(light, surfaceScale, input, cropRect),
    fKD(kd)
{
}

SkFlattenable* SkDiffuseLightingImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkAutoTUnref<SkImageFilterLight> light(SkImageFilterLight::UnflattenLight(buffer));
    SkScalar surfaceScale = buffer.readScalar();
    SkScalar kd = buffer.readScalar();
    return Create(light, surfaceScale, kd, common.getInput(0), &common.cropRect());
}

void SkDiffuseLightingImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fKD);
}

bool SkDiffuseLightingImageFilter::onFilterImage(Proxy* proxy,
                                                 const SkBitmap& source,
                                                 const Context& ctx,
                                                 SkBitmap* dst,
                                                 SkIPoint* offset) const {
    SkBitmap src = source;
    SkIPoint srcOffset = SkIPoint::Make(0, 0);
    if (!this->filterInput(0, proxy, source, ctx, &src, &srcOffset)) {
        return false;
    }

    if (src.colorType() != kN32_SkColorType) {
        return false;
    }
    SkIRect bounds;
    if (!this->applyCropRect(ctx, proxy, src, &srcOffset, &bounds, &src)) {
        return false;
    }

    if (bounds.width() < 2 || bounds.height() < 2) {
        return false;
    }

    SkAutoLockPixels alp(src);
    if (!src.getPixels()) {
        return false;
    }

    SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(bounds.width(), bounds.height()));
    if (!device) {
        return false;
    }
    *dst = device->accessBitmap(false);
    SkAutoLockPixels alp_dst(*dst);

    SkMatrix matrix(ctx.ctm());
    matrix.postTranslate(SkIntToScalar(-srcOffset.x()), SkIntToScalar(-srcOffset.y()));
    SkAutoTUnref<SkImageFilterLight> transformedLight(light()->transform(matrix));

    DiffuseLightingType lightingType(fKD);
    offset->fX = bounds.left();
    offset->fY = bounds.top();
    bounds.offset(-srcOffset);
    switch (transformedLight->type()) {
        case SkImageFilterLight::kDistant_LightType:
            lightBitmap<DiffuseLightingType, SkDistantLight>(lightingType,
                                                             transformedLight,
                                                             src,
                                                             dst,
                                                             surfaceScale(),
                                                             bounds);
            break;
        case SkImageFilterLight::kPoint_LightType:
            lightBitmap<DiffuseLightingType, SkPointLight>(lightingType,
                                                           transformedLight,
                                                           src,
                                                           dst,
                                                           surfaceScale(),
                                                           bounds);
            break;
        case SkImageFilterLight::kSpot_LightType:
            lightBitmap<DiffuseLightingType, SkSpotLight>(lightingType,
                                                          transformedLight,
                                                          src,
                                                          dst,
                                                          surfaceScale(),
                                                          bounds);
            break;
    }

    return true;
}

#ifndef SK_IGNORE_TO_STRING
void SkDiffuseLightingImageFilter::toString(SkString* str) const {
    str->appendf("SkDiffuseLightingImageFilter: (");
    str->appendf("kD: %f\n", fKD);
    str->append(")");
}
#endif

#if SK_SUPPORT_GPU
GrFragmentProcessor* SkDiffuseLightingImageFilter::getFragmentProcessor(
                                                   GrTexture* texture,
                                                   const SkMatrix& matrix,
                                                   const SkIRect&,
                                                   BoundaryMode boundaryMode
) const {
    SkScalar scale = SkScalarMul(this->surfaceScale(), SkIntToScalar(255));
    return GrDiffuseLightingEffect::Create(texture, this->light(), scale, matrix, this->kd(),
                                           boundaryMode);
}
#endif

///////////////////////////////////////////////////////////////////////////////

SkImageFilter* SkSpecularLightingImageFilter::Create(SkImageFilterLight* light,
                                                     SkScalar surfaceScale,
                                                     SkScalar ks,
                                                     SkScalar shininess,
                                                     SkImageFilter* input,
                                                     const CropRect* cropRect) {
    if (nullptr == light) {
        return nullptr;
    }
    if (!SkScalarIsFinite(surfaceScale) || !SkScalarIsFinite(ks) || !SkScalarIsFinite(shininess)) {
        return nullptr;
    }
    // According to the spec, ks can be any non-negative number :
    // http://www.w3.org/TR/SVG/filters.html#feSpecularLightingElement
    if (ks < 0) {
        return nullptr;
    }
    return new SkSpecularLightingImageFilter(light, surfaceScale, ks, shininess, input, cropRect);
}

SkSpecularLightingImageFilter::SkSpecularLightingImageFilter(SkImageFilterLight* light,
                                                             SkScalar surfaceScale,
                                                             SkScalar ks,
                                                             SkScalar shininess,
                                                             SkImageFilter* input,
                                                             const CropRect* cropRect)
  : INHERITED(light, surfaceScale, input, cropRect),
    fKS(ks),
    fShininess(shininess)
{
}

SkFlattenable* SkSpecularLightingImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkAutoTUnref<SkImageFilterLight> light(SkImageFilterLight::UnflattenLight(buffer));
    SkScalar surfaceScale = buffer.readScalar();
    SkScalar ks = buffer.readScalar();
    SkScalar shine = buffer.readScalar();
    return Create(light, surfaceScale, ks, shine, common.getInput(0), &common.cropRect());
}

void SkSpecularLightingImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fKS);
    buffer.writeScalar(fShininess);
}

bool SkSpecularLightingImageFilter::onFilterImage(Proxy* proxy,
                                                  const SkBitmap& source,
                                                  const Context& ctx,
                                                  SkBitmap* dst,
                                                  SkIPoint* offset) const {
    SkBitmap src = source;
    SkIPoint srcOffset = SkIPoint::Make(0, 0);
    if (!this->filterInput(0, proxy, source, ctx, &src, &srcOffset)) {
        return false;
    }

    if (src.colorType() != kN32_SkColorType) {
        return false;
    }

    SkIRect bounds;
    if (!this->applyCropRect(ctx, proxy, src, &srcOffset, &bounds, &src)) {
        return false;
    }

    if (bounds.width() < 2 || bounds.height() < 2) {
        return false;
    }

    SkAutoLockPixels alp(src);
    if (!src.getPixels()) {
        return false;
    }

    SkAutoTUnref<SkBaseDevice> device(proxy->createDevice(bounds.width(), bounds.height()));
    if (!device) {
        return false;
    }
    *dst = device->accessBitmap(false);
    SkAutoLockPixels alp_dst(*dst);

    SpecularLightingType lightingType(fKS, fShininess);
    offset->fX = bounds.left();
    offset->fY = bounds.top();
    SkMatrix matrix(ctx.ctm());
    matrix.postTranslate(SkIntToScalar(-srcOffset.x()), SkIntToScalar(-srcOffset.y()));
    SkAutoTUnref<SkImageFilterLight> transformedLight(light()->transform(matrix));
    bounds.offset(-srcOffset);
    switch (transformedLight->type()) {
        case SkImageFilterLight::kDistant_LightType:
            lightBitmap<SpecularLightingType, SkDistantLight>(lightingType,
                                                              transformedLight,
                                                              src,
                                                              dst,
                                                              surfaceScale(),
                                                              bounds);
            break;
        case SkImageFilterLight::kPoint_LightType:
            lightBitmap<SpecularLightingType, SkPointLight>(lightingType,
                                                            transformedLight,
                                                            src,
                                                            dst,
                                                            surfaceScale(),
                                                            bounds);
            break;
        case SkImageFilterLight::kSpot_LightType:
            lightBitmap<SpecularLightingType, SkSpotLight>(lightingType,
                                                           transformedLight,
                                                           src,
                                                           dst,
                                                           surfaceScale(),
                                                           bounds);
            break;
    }
    return true;
}

#ifndef SK_IGNORE_TO_STRING
void SkSpecularLightingImageFilter::toString(SkString* str) const {
    str->appendf("SkSpecularLightingImageFilter: (");
    str->appendf("kS: %f shininess: %f", fKS, fShininess);
    str->append(")");
}
#endif

#if SK_SUPPORT_GPU
GrFragmentProcessor* SkSpecularLightingImageFilter::getFragmentProcessor(
                                                    GrTexture* texture,
                                                    const SkMatrix& matrix,
                                                    const SkIRect&,
                                                    BoundaryMode boundaryMode) const {
    SkScalar scale = SkScalarMul(this->surfaceScale(), SkIntToScalar(255));
    return GrSpecularLightingEffect::Create(texture, this->light(), scale, matrix, this->ks(),
                                            this->shininess(), boundaryMode);
}
#endif

///////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

namespace {
SkPoint3 random_point3(SkRandom* random) {
    return SkPoint3::Make(SkScalarToFloat(random->nextSScalar1()),
                          SkScalarToFloat(random->nextSScalar1()),
                          SkScalarToFloat(random->nextSScalar1()));
}

SkImageFilterLight* create_random_light(SkRandom* random) {
    int type = random->nextULessThan(3);
    switch (type) {
        case 0: {
            return new SkDistantLight(random_point3(random), random->nextU());
        }
        case 1: {
            return new SkPointLight(random_point3(random), random->nextU());
        }
        case 2: {
            return new SkSpotLight(random_point3(random), random_point3(random),
                                   random->nextUScalar1(), random->nextUScalar1(), random->nextU());
        }
        default:
            SkFAIL("Unexpected value.");
            return nullptr;
    }
}

SkString emitNormalFunc(BoundaryMode mode,
                        const char* pointToNormalName,
                        const char* sobelFuncName) {
    SkString result;
    switch (mode) {
    case kTopLeft_BoundaryMode:
        result.printf("\treturn %s(%s(0.0, 0.0, m[4], m[5], m[7], m[8], %g),\n"
                      "\t          %s(0.0, 0.0, m[4], m[7], m[5], m[8], %g),\n"
                      "\t          surfaceScale);\n",
                      pointToNormalName, sobelFuncName, gTwoThirds,
                                         sobelFuncName, gTwoThirds);
        break;
    case kTop_BoundaryMode:
        result.printf("\treturn %s(%s(0.0, 0.0, m[3], m[5], m[6], m[8], %g),\n"
                      "\t          %s(0.0, 0.0, m[4], m[7], m[5], m[8], %g),\n"
                      "\t          surfaceScale);\n",
                      pointToNormalName, sobelFuncName, gOneThird,
                                         sobelFuncName, gOneHalf);
        break;
    case kTopRight_BoundaryMode:
        result.printf("\treturn %s(%s( 0.0,  0.0, m[3], m[4], m[6], m[7], %g),\n"
                      "\t          %s(m[3], m[6], m[4], m[7],  0.0,  0.0, %g),\n"
                      "\t          surfaceScale);\n",
                      pointToNormalName, sobelFuncName, gTwoThirds,
                                         sobelFuncName, gTwoThirds);
        break;
    case kLeft_BoundaryMode:
        result.printf("\treturn %s(%s(m[1], m[2], m[4], m[5], m[7], m[8], %g),\n"
                      "\t          %s( 0.0,  0.0, m[1], m[7], m[2], m[8], %g),\n"
                      "\t          surfaceScale);\n",
                      pointToNormalName, sobelFuncName, gOneHalf,
                                         sobelFuncName, gOneThird);
        break;
    case kInterior_BoundaryMode:
        result.printf("\treturn %s(%s(m[0], m[2], m[3], m[5], m[6], m[8], %g),\n"
                      "\t          %s(m[0], m[6], m[1], m[7], m[2], m[8], %g),\n"
                      "\t          surfaceScale);\n",
                      pointToNormalName, sobelFuncName, gOneQuarter,
                                         sobelFuncName, gOneQuarter);
        break;
    case kRight_BoundaryMode:
        result.printf("\treturn %s(%s(m[0], m[1], m[3], m[4], m[6], m[7], %g),\n"
                      "\t          %s(m[0], m[6], m[1], m[7],  0.0,  0.0, %g),\n"
                      "\t          surfaceScale);\n",
                      pointToNormalName, sobelFuncName, gOneHalf,
                                         sobelFuncName, gOneThird);
        break;
    case kBottomLeft_BoundaryMode:
        result.printf("\treturn %s(%s(m[1], m[2], m[4], m[5],  0.0,  0.0, %g),\n"
                      "\t          %s( 0.0,  0.0, m[1], m[4], m[2], m[5], %g),\n"
                      "\t          surfaceScale);\n",
                      pointToNormalName, sobelFuncName, gTwoThirds,
                                         sobelFuncName, gTwoThirds);
        break;
    case kBottom_BoundaryMode:
        result.printf("\treturn %s(%s(m[0], m[2], m[3], m[5],  0.0,  0.0, %g),\n"
                      "\t          %s(m[0], m[3], m[1], m[4], m[2], m[5], %g),\n"
                      "\t          surfaceScale);\n",
                      pointToNormalName, sobelFuncName, gOneThird,
                                         sobelFuncName, gOneHalf);
        break;
    case kBottomRight_BoundaryMode:
        result.printf("\treturn %s(%s(m[0], m[1], m[3], m[4],  0.0,  0.0, %g),\n"
                      "\t          %s(m[0], m[3], m[1], m[4],  0.0,  0.0, %g),\n"
                      "\t          surfaceScale);\n",
                      pointToNormalName, sobelFuncName, gTwoThirds,
                                         sobelFuncName, gTwoThirds);
        break;
    default:
        SkASSERT(false);
        break;
    }
    return result;
}

}

class GrGLLightingEffect  : public GrGLSLFragmentProcessor {
public:
    GrGLLightingEffect(const GrProcessor&);
    virtual ~GrGLLightingEffect();

    void emitCode(EmitArgs&) override;

    static inline void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder* b);

protected:
    /**
     * Subclasses of GrGLLightingEffect must call INHERITED::onSetData();
     */
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

    virtual void emitLightFunc(GrGLSLUniformHandler*,
                               GrGLSLFragmentBuilder*,
                               SkString* funcName) = 0;

private:
    typedef GrGLSLFragmentProcessor INHERITED;

    UniformHandle       fImageIncrementUni;
    UniformHandle       fSurfaceScaleUni;
    GrGLLight*          fLight;
    BoundaryMode        fBoundaryMode;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLDiffuseLightingEffect  : public GrGLLightingEffect {
public:
    GrGLDiffuseLightingEffect(const GrProcessor&);
    void emitLightFunc(GrGLSLUniformHandler*, GrGLSLFragmentBuilder*, SkString* funcName) override;

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

private:
    typedef GrGLLightingEffect INHERITED;

    UniformHandle   fKDUni;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLSpecularLightingEffect  : public GrGLLightingEffect {
public:
    GrGLSpecularLightingEffect(const GrProcessor&);
    void emitLightFunc(GrGLSLUniformHandler*, GrGLSLFragmentBuilder*, SkString* funcName) override;

protected:
    void onSetData(const GrGLSLProgramDataManager&, const GrProcessor&) override;

private:
    typedef GrGLLightingEffect INHERITED;

    UniformHandle   fKSUni;
    UniformHandle   fShininessUni;
};

///////////////////////////////////////////////////////////////////////////////

GrLightingEffect::GrLightingEffect(GrTexture* texture,
                                   const SkImageFilterLight* light,
                                   SkScalar surfaceScale,
                                   const SkMatrix& matrix,
                                   BoundaryMode boundaryMode)
    : INHERITED(texture, GrCoordTransform::MakeDivByTextureWHMatrix(texture))
    , fLight(light)
    , fSurfaceScale(surfaceScale)
    , fFilterMatrix(matrix)
    , fBoundaryMode(boundaryMode) {
    fLight->ref();
    if (light->requiresFragmentPosition()) {
        this->setWillReadFragmentPosition();
    }
}

GrLightingEffect::~GrLightingEffect() {
    fLight->unref();
}

bool GrLightingEffect::onIsEqual(const GrFragmentProcessor& sBase) const {
    const GrLightingEffect& s = sBase.cast<GrLightingEffect>();
    return fLight->isEqual(*s.fLight) &&
           fSurfaceScale == s.fSurfaceScale &&
           fBoundaryMode == s.fBoundaryMode;
}

///////////////////////////////////////////////////////////////////////////////

GrDiffuseLightingEffect::GrDiffuseLightingEffect(GrTexture* texture,
                                                 const SkImageFilterLight* light,
                                                 SkScalar surfaceScale,
                                                 const SkMatrix& matrix,
                                                 SkScalar kd,
                                                 BoundaryMode boundaryMode)
    : INHERITED(texture, light, surfaceScale, matrix, boundaryMode), fKD(kd) {
    this->initClassID<GrDiffuseLightingEffect>();
}

bool GrDiffuseLightingEffect::onIsEqual(const GrFragmentProcessor& sBase) const {
    const GrDiffuseLightingEffect& s = sBase.cast<GrDiffuseLightingEffect>();
    return INHERITED::onIsEqual(sBase) &&
            this->kd() == s.kd();
}

void GrDiffuseLightingEffect::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                                    GrProcessorKeyBuilder* b) const {
    GrGLDiffuseLightingEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* GrDiffuseLightingEffect::onCreateGLSLInstance() const {
    return new GrGLDiffuseLightingEffect(*this);
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrDiffuseLightingEffect);

const GrFragmentProcessor* GrDiffuseLightingEffect::TestCreate(GrProcessorTestData* d) {
    SkScalar surfaceScale = d->fRandom->nextSScalar1();
    SkScalar kd = d->fRandom->nextUScalar1();
    SkAutoTUnref<SkImageFilterLight> light(create_random_light(d->fRandom));
    SkMatrix matrix;
    for (int i = 0; i < 9; i++) {
        matrix[i] = d->fRandom->nextUScalar1();
    }
    BoundaryMode mode = static_cast<BoundaryMode>(d->fRandom->nextU() % kBoundaryModeCount);
    return GrDiffuseLightingEffect::Create(d->fTextures[GrProcessorUnitTest::kAlphaTextureIdx],
                                           light, surfaceScale, matrix, kd, mode);
}


///////////////////////////////////////////////////////////////////////////////

GrGLLightingEffect::GrGLLightingEffect(const GrProcessor& fp) {
    const GrLightingEffect& m = fp.cast<GrLightingEffect>();
    fLight = m.light()->createGLLight();
    fBoundaryMode = m.boundaryMode();
}

GrGLLightingEffect::~GrGLLightingEffect() {
    delete fLight;
}

void GrGLLightingEffect::emitCode(EmitArgs& args) {
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    fImageIncrementUni = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                                    kVec2f_GrSLType, kDefault_GrSLPrecision,
                                                    "ImageIncrement");
    fSurfaceScaleUni = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                                  kFloat_GrSLType, kDefault_GrSLPrecision,
                                                  "SurfaceScale");
    fLight->emitLightColorUniform(uniformHandler);
    GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;
    SkString lightFunc;
    this->emitLightFunc(uniformHandler, fragBuilder, &lightFunc);
    static const GrGLSLShaderVar gSobelArgs[] =  {
        GrGLSLShaderVar("a", kFloat_GrSLType),
        GrGLSLShaderVar("b", kFloat_GrSLType),
        GrGLSLShaderVar("c", kFloat_GrSLType),
        GrGLSLShaderVar("d", kFloat_GrSLType),
        GrGLSLShaderVar("e", kFloat_GrSLType),
        GrGLSLShaderVar("f", kFloat_GrSLType),
        GrGLSLShaderVar("scale", kFloat_GrSLType),
    };
    SkString sobelFuncName;
    SkString coords2D = fragBuilder->ensureFSCoords2D(args.fCoords, 0);

    fragBuilder->emitFunction(kFloat_GrSLType,
                              "sobel",
                              SK_ARRAY_COUNT(gSobelArgs),
                              gSobelArgs,
                              "\treturn (-a + b - 2.0 * c + 2.0 * d -e + f) * scale;\n",
                              &sobelFuncName);
    static const GrGLSLShaderVar gPointToNormalArgs[] =  {
        GrGLSLShaderVar("x", kFloat_GrSLType),
        GrGLSLShaderVar("y", kFloat_GrSLType),
        GrGLSLShaderVar("scale", kFloat_GrSLType),
    };
    SkString pointToNormalName;
    fragBuilder->emitFunction(kVec3f_GrSLType,
                              "pointToNormal",
                              SK_ARRAY_COUNT(gPointToNormalArgs),
                              gPointToNormalArgs,
                              "\treturn normalize(vec3(-x * scale, -y * scale, 1));\n",
                              &pointToNormalName);

    static const GrGLSLShaderVar gInteriorNormalArgs[] =  {
        GrGLSLShaderVar("m", kFloat_GrSLType, 9),
        GrGLSLShaderVar("surfaceScale", kFloat_GrSLType),
    };
    SkString normalBody = emitNormalFunc(fBoundaryMode,
                                         pointToNormalName.c_str(),
                                         sobelFuncName.c_str());
    SkString normalName;
    fragBuilder->emitFunction(kVec3f_GrSLType,
                              "normal",
                              SK_ARRAY_COUNT(gInteriorNormalArgs),
                              gInteriorNormalArgs,
                              normalBody.c_str(),
                              &normalName);

    fragBuilder->codeAppendf("\t\tvec2 coord = %s;\n", coords2D.c_str());
    fragBuilder->codeAppend("\t\tfloat m[9];\n");

    const char* imgInc = uniformHandler->getUniformCStr(fImageIncrementUni);
    const char* surfScale = uniformHandler->getUniformCStr(fSurfaceScaleUni);

    int index = 0;
    for (int dy = 1; dy >= -1; dy--) {
        for (int dx = -1; dx <= 1; dx++) {
            SkString texCoords;
            texCoords.appendf("coord + vec2(%d, %d) * %s", dx, dy, imgInc);
            fragBuilder->codeAppendf("\t\tm[%d] = ", index++);
            fragBuilder->appendTextureLookup(args.fSamplers[0], texCoords.c_str());
            fragBuilder->codeAppend(".a;\n");
        }
    }
    fragBuilder->codeAppend("\t\tvec3 surfaceToLight = ");
    SkString arg;
    arg.appendf("%s * m[4]", surfScale);
    fLight->emitSurfaceToLight(uniformHandler, fragBuilder, arg.c_str());
    fragBuilder->codeAppend(";\n");
    fragBuilder->codeAppendf("\t\t%s = %s(%s(m, %s), surfaceToLight, ",
                             args.fOutputColor, lightFunc.c_str(), normalName.c_str(), surfScale);
    fLight->emitLightColor(uniformHandler, fragBuilder, "surfaceToLight");
    fragBuilder->codeAppend(");\n");
    SkString modulate;
    GrGLSLMulVarBy4f(&modulate, args.fOutputColor, args.fInputColor);
    fragBuilder->codeAppend(modulate.c_str());
}

void GrGLLightingEffect::GenKey(const GrProcessor& proc,
                                const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) {
    const GrLightingEffect& lighting = proc.cast<GrLightingEffect>();
    b->add32(lighting.boundaryMode() << 2 | lighting.light()->type());
}

void GrGLLightingEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                   const GrProcessor& proc) {
    const GrLightingEffect& lighting = proc.cast<GrLightingEffect>();
    GrTexture* texture = lighting.texture(0);
    float ySign = texture->origin() == kTopLeft_GrSurfaceOrigin ? -1.0f : 1.0f;
    pdman.set2f(fImageIncrementUni, 1.0f / texture->width(), ySign / texture->height());
    pdman.set1f(fSurfaceScaleUni, lighting.surfaceScale());
    SkAutoTUnref<SkImageFilterLight> transformedLight(
                                            lighting.light()->transform(lighting.filterMatrix()));
    fLight->setData(pdman, transformedLight);
}

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

GrGLDiffuseLightingEffect::GrGLDiffuseLightingEffect(const GrProcessor& proc)
    : INHERITED(proc) {
}

void GrGLDiffuseLightingEffect::emitLightFunc(GrGLSLUniformHandler* uniformHandler,
                                              GrGLSLFragmentBuilder* fragBuilder,
                                              SkString* funcName) {
    const char* kd;
    fKDUni = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                 kFloat_GrSLType, kDefault_GrSLPrecision,
                                 "KD", &kd);

    static const GrGLSLShaderVar gLightArgs[] = {
        GrGLSLShaderVar("normal", kVec3f_GrSLType),
        GrGLSLShaderVar("surfaceToLight", kVec3f_GrSLType),
        GrGLSLShaderVar("lightColor", kVec3f_GrSLType)
    };
    SkString lightBody;
    lightBody.appendf("\tfloat colorScale = %s * dot(normal, surfaceToLight);\n", kd);
    lightBody.appendf("\treturn vec4(lightColor * clamp(colorScale, 0.0, 1.0), 1.0);\n");
    fragBuilder->emitFunction(kVec4f_GrSLType,
                              "light",
                              SK_ARRAY_COUNT(gLightArgs),
                              gLightArgs,
                              lightBody.c_str(),
                              funcName);
}

void GrGLDiffuseLightingEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                          const GrProcessor& proc) {
    INHERITED::onSetData(pdman, proc);
    const GrDiffuseLightingEffect& diffuse = proc.cast<GrDiffuseLightingEffect>();
    pdman.set1f(fKDUni, diffuse.kd());
}

///////////////////////////////////////////////////////////////////////////////

GrSpecularLightingEffect::GrSpecularLightingEffect(GrTexture* texture,
                                                   const SkImageFilterLight* light,
                                                   SkScalar surfaceScale,
                                                   const SkMatrix& matrix,
                                                   SkScalar ks,
                                                   SkScalar shininess,
                                                   BoundaryMode boundaryMode)
    : INHERITED(texture, light, surfaceScale, matrix, boundaryMode)
    , fKS(ks)
    , fShininess(shininess) {
    this->initClassID<GrSpecularLightingEffect>();
}

bool GrSpecularLightingEffect::onIsEqual(const GrFragmentProcessor& sBase) const {
    const GrSpecularLightingEffect& s = sBase.cast<GrSpecularLightingEffect>();
    return INHERITED::onIsEqual(sBase) &&
           this->ks() == s.ks() &&
           this->shininess() == s.shininess();
}

void GrSpecularLightingEffect::onGetGLSLProcessorKey(const GrGLSLCaps& caps,
                                                     GrProcessorKeyBuilder* b) const {
    GrGLSpecularLightingEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* GrSpecularLightingEffect::onCreateGLSLInstance() const {
    return new GrGLSpecularLightingEffect(*this);
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrSpecularLightingEffect);

const GrFragmentProcessor* GrSpecularLightingEffect::TestCreate(GrProcessorTestData* d) {
    SkScalar surfaceScale = d->fRandom->nextSScalar1();
    SkScalar ks = d->fRandom->nextUScalar1();
    SkScalar shininess = d->fRandom->nextUScalar1();
    SkAutoTUnref<SkImageFilterLight> light(create_random_light(d->fRandom));
    SkMatrix matrix;
    for (int i = 0; i < 9; i++) {
        matrix[i] = d->fRandom->nextUScalar1();
    }
    BoundaryMode mode = static_cast<BoundaryMode>(d->fRandom->nextU() % kBoundaryModeCount);
    return GrSpecularLightingEffect::Create(d->fTextures[GrProcessorUnitTest::kAlphaTextureIdx],
                                            light, surfaceScale, matrix, ks, shininess, mode);
}

///////////////////////////////////////////////////////////////////////////////

GrGLSpecularLightingEffect::GrGLSpecularLightingEffect(const GrProcessor& proc)
    : INHERITED(proc) {
}

void GrGLSpecularLightingEffect::emitLightFunc(GrGLSLUniformHandler* uniformHandler,
                                               GrGLSLFragmentBuilder* fragBuilder,
                                               SkString* funcName) {
    const char* ks;
    const char* shininess;

    fKSUni = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                        kFloat_GrSLType, kDefault_GrSLPrecision, "KS", &ks);
    fShininessUni = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                               kFloat_GrSLType,
                                               kDefault_GrSLPrecision,
                                               "Shininess",
                                               &shininess);

    static const GrGLSLShaderVar gLightArgs[] = {
        GrGLSLShaderVar("normal", kVec3f_GrSLType),
        GrGLSLShaderVar("surfaceToLight", kVec3f_GrSLType),
        GrGLSLShaderVar("lightColor", kVec3f_GrSLType)
    };
    SkString lightBody;
    lightBody.appendf("\tvec3 halfDir = vec3(normalize(surfaceToLight + vec3(0, 0, 1)));\n");
    lightBody.appendf("\tfloat colorScale = %s * pow(dot(normal, halfDir), %s);\n", ks, shininess);
    lightBody.appendf("\tvec3 color = lightColor * clamp(colorScale, 0.0, 1.0);\n");
    lightBody.appendf("\treturn vec4(color, max(max(color.r, color.g), color.b));\n");
    fragBuilder->emitFunction(kVec4f_GrSLType,
                              "light",
                              SK_ARRAY_COUNT(gLightArgs),
                              gLightArgs,
                              lightBody.c_str(),
                              funcName);
}

void GrGLSpecularLightingEffect::onSetData(const GrGLSLProgramDataManager& pdman,
                                           const GrProcessor& effect) {
    INHERITED::onSetData(pdman, effect);
    const GrSpecularLightingEffect& spec = effect.cast<GrSpecularLightingEffect>();
    pdman.set1f(fKSUni, spec.ks());
    pdman.set1f(fShininessUni, spec.shininess());
}

///////////////////////////////////////////////////////////////////////////////
void GrGLLight::emitLightColorUniform(GrGLSLUniformHandler* uniformHandler) {
    fColorUni = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                           kVec3f_GrSLType, kDefault_GrSLPrecision,
                                           "LightColor");
}

void GrGLLight::emitLightColor(GrGLSLUniformHandler* uniformHandler,
                               GrGLSLFragmentBuilder* fragBuilder,
                               const char *surfaceToLight) {
    fragBuilder->codeAppend(uniformHandler->getUniformCStr(this->lightColorUni()));
}

void GrGLLight::setData(const GrGLSLProgramDataManager& pdman,
                        const SkImageFilterLight* light) const {
    setUniformPoint3(pdman, fColorUni,
                     light->color().makeScale(SkScalarInvert(SkIntToScalar(255))));
}

///////////////////////////////////////////////////////////////////////////////

void GrGLDistantLight::setData(const GrGLSLProgramDataManager& pdman,
                               const SkImageFilterLight* light) const {
    INHERITED::setData(pdman, light);
    SkASSERT(light->type() == SkImageFilterLight::kDistant_LightType);
    const SkDistantLight* distantLight = static_cast<const SkDistantLight*>(light);
    setUniformNormal3(pdman, fDirectionUni, distantLight->direction());
}

void GrGLDistantLight::emitSurfaceToLight(GrGLSLUniformHandler* uniformHandler,
                                          GrGLSLFragmentBuilder* fragBuilder,
                                          const char* z) {
    const char* dir;
    fDirectionUni = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                               kVec3f_GrSLType, kDefault_GrSLPrecision,
                                               "LightDirection", &dir);
    fragBuilder->codeAppend(dir);
}

///////////////////////////////////////////////////////////////////////////////

void GrGLPointLight::setData(const GrGLSLProgramDataManager& pdman,
                             const SkImageFilterLight* light) const {
    INHERITED::setData(pdman, light);
    SkASSERT(light->type() == SkImageFilterLight::kPoint_LightType);
    const SkPointLight* pointLight = static_cast<const SkPointLight*>(light);
    setUniformPoint3(pdman, fLocationUni, pointLight->location());
}

void GrGLPointLight::emitSurfaceToLight(GrGLSLUniformHandler* uniformHandler,
                                        GrGLSLFragmentBuilder* fragBuilder,
                                        const char* z) {
    const char* loc;
    fLocationUni = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                              kVec3f_GrSLType, kDefault_GrSLPrecision,
                                              "LightLocation", &loc);
    fragBuilder->codeAppendf("normalize(%s - vec3(%s.xy, %s))",
                             loc, fragBuilder->fragmentPosition(), z);
}

///////////////////////////////////////////////////////////////////////////////

void GrGLSpotLight::setData(const GrGLSLProgramDataManager& pdman,
                            const SkImageFilterLight* light) const {
    INHERITED::setData(pdman, light);
    SkASSERT(light->type() == SkImageFilterLight::kSpot_LightType);
    const SkSpotLight* spotLight = static_cast<const SkSpotLight *>(light);
    setUniformPoint3(pdman, fLocationUni, spotLight->location());
    pdman.set1f(fExponentUni, spotLight->specularExponent());
    pdman.set1f(fCosInnerConeAngleUni, spotLight->cosInnerConeAngle());
    pdman.set1f(fCosOuterConeAngleUni, spotLight->cosOuterConeAngle());
    pdman.set1f(fConeScaleUni, spotLight->coneScale());
    setUniformNormal3(pdman, fSUni, spotLight->s());
}

void GrGLSpotLight::emitSurfaceToLight(GrGLSLUniformHandler* uniformHandler,
                                       GrGLSLFragmentBuilder* fragBuilder,
                                       const char* z) {
    const char* location;
    fLocationUni = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                              kVec3f_GrSLType, kDefault_GrSLPrecision,
                                              "LightLocation", &location);

    fragBuilder->codeAppendf("normalize(%s - vec3(%s.xy, %s))",
                             location, fragBuilder->fragmentPosition(), z);
}

void GrGLSpotLight::emitLightColor(GrGLSLUniformHandler* uniformHandler,
                                   GrGLSLFragmentBuilder* fragBuilder,
                                   const char *surfaceToLight) {

    const char* color = uniformHandler->getUniformCStr(this->lightColorUni()); // created by parent class.

    const char* exponent;
    const char* cosInner;
    const char* cosOuter;
    const char* coneScale;
    const char* s;
    fExponentUni = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                              kFloat_GrSLType, kDefault_GrSLPrecision,
                                              "Exponent", &exponent);
    fCosInnerConeAngleUni = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                                       kFloat_GrSLType, kDefault_GrSLPrecision,
                                                       "CosInnerConeAngle", &cosInner);
    fCosOuterConeAngleUni = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                                       kFloat_GrSLType, kDefault_GrSLPrecision,
                                                       "CosOuterConeAngle", &cosOuter);
    fConeScaleUni = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                               kFloat_GrSLType, kDefault_GrSLPrecision,
                                               "ConeScale", &coneScale);
    fSUni = uniformHandler->addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                       kVec3f_GrSLType, kDefault_GrSLPrecision, "S", &s);

    static const GrGLSLShaderVar gLightColorArgs[] = {
        GrGLSLShaderVar("surfaceToLight", kVec3f_GrSLType)
    };
    SkString lightColorBody;
    lightColorBody.appendf("\tfloat cosAngle = -dot(surfaceToLight, %s);\n", s);
    lightColorBody.appendf("\tif (cosAngle < %s) {\n", cosOuter);
    lightColorBody.appendf("\t\treturn vec3(0);\n");
    lightColorBody.appendf("\t}\n");
    lightColorBody.appendf("\tfloat scale = pow(cosAngle, %s);\n", exponent);
    lightColorBody.appendf("\tif (cosAngle < %s) {\n", cosInner);
    lightColorBody.appendf("\t\treturn %s * scale * (cosAngle - %s) * %s;\n",
                           color, cosOuter, coneScale);
    lightColorBody.appendf("\t}\n");
    lightColorBody.appendf("\treturn %s;\n", color);
    fragBuilder->emitFunction(kVec3f_GrSLType,
                              "lightColor",
                              SK_ARRAY_COUNT(gLightColorArgs),
                              gLightColorArgs,
                              lightColorBody.c_str(),
                              &fLightColorFunc);

    fragBuilder->codeAppendf("%s(%s)", fLightColorFunc.c_str(), surfaceToLight);
}

#endif

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkLightingImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDiffuseLightingImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkSpecularLightingImageFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
