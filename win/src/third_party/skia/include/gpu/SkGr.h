
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGr_DEFINED
#define SkGr_DEFINED

#include "include/gpu/GrColor.h"
#include "include/gpu/GrTextureAccess.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkFilterQuality.h"
#include "include/core/SkImageInfo.h"

class GrContext;
class GrTexture;
class GrTextureParams;
class SkBitmap;

////////////////////////////////////////////////////////////////////////////////
// Sk to Gr Type conversions

static inline GrColor SkColorToPremulGrColor(SkColor c) {
    SkPMColor pm = SkPreMultiplyColor(c);
    unsigned r = SkGetPackedR32(pm);
    unsigned g = SkGetPackedG32(pm);
    unsigned b = SkGetPackedB32(pm);
    unsigned a = SkGetPackedA32(pm);
    return GrColorPackRGBA(r, g, b, a);
}

static inline GrColor SkColorToUnpremulGrColor(SkColor c) {
    unsigned r = SkColorGetR(c);
    unsigned g = SkColorGetG(c);
    unsigned b = SkColorGetB(c);
    unsigned a = SkColorGetA(c);
    return GrColorPackRGBA(r, g, b, a);
}

static inline GrColor SkColorToOpaqueGrColor(SkColor c) {
    unsigned r = SkColorGetR(c);
    unsigned g = SkColorGetG(c);
    unsigned b = SkColorGetB(c);
    return GrColorPackRGBA(r, g, b, 0xFF);
}

/** Replicates the SkColor's alpha to all four channels of the GrColor. */
static inline GrColor SkColorAlphaToGrColor(SkColor c) {
    U8CPU a = SkColorGetA(c);
    return GrColorPackRGBA(a, a, a, a);
}

static inline SkPMColor GrColorToSkPMColor(GrColor c) {
    GrColorIsPMAssert(c);
    return SkPackARGB32(GrColorUnpackA(c), GrColorUnpackR(c), GrColorUnpackG(c), GrColorUnpackB(c));
}

static inline GrColor SkPMColorToGrColor(SkPMColor c) {
    return GrColorPackRGBA(SkGetPackedR32(c), SkGetPackedG32(c), SkGetPackedB32(c),
                           SkGetPackedA32(c));
}

////////////////////////////////////////////////////////////////////////////////
/** Returns a texture representing the bitmap that is compatible with the GrTextureParams. The
    texture is inserted into the cache (unless the bitmap is marked volatile) and can be
    retrieved again via this function. */
GrTexture* GrRefCachedBitmapTexture(GrContext*, const SkBitmap&, const GrTextureParams&);

// TODO: Move SkImageInfo2GrPixelConfig to SkGrPriv.h (requires cleanup to SkWindow its subclasses).
GrPixelConfig SkImageInfo2GrPixelConfig(SkColorType, SkAlphaType, SkColorProfileType);

static inline GrPixelConfig SkImageInfo2GrPixelConfig(const SkImageInfo& info) {
    return SkImageInfo2GrPixelConfig(info.colorType(), info.alphaType(), info.profileType());
}

GrTextureParams::FilterMode GrSkFilterQualityToGrFilterMode(SkFilterQuality paintFilterQuality,
                                                            const SkMatrix& viewM,
                                                            const SkMatrix& localM,
                                                            bool* doBicubic);

////////////////////////////////////////////////////////////////////////////////

SkImageInfo GrMakeInfoFromTexture(GrTexture* tex, int w, int h, bool isOpaque);

// Using the dreaded SkGrPixelRef ...
SK_API void GrWrapTextureInBitmap(GrTexture* src, int w, int h, bool isOpaque,
                                  SkBitmap* dst);

#endif
