
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkCGUtils_DEFINED
#define SkCGUtils_DEFINED

#include "include/core/SkSize.h"
#include "include/core/SkImageInfo.h"

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#ifdef SK_BUILD_FOR_MAC
#include <ApplicationServices/ApplicationServices.h>
#endif

#ifdef SK_BUILD_FOR_IOS
#include <CoreGraphics/CoreGraphics.h>
#endif

class SkBitmap;
class SkData;
class SkStream;

/**
 *  Given a CGImage, allocate an SkBitmap and copy the image's pixels into it. If scaleToFit is not
 *  null, use it to determine the size of the bitmap, and scale the image to fill the bitmap.
 *  Otherwise use the image's width/height.
 *
 *  On failure, return false, and leave bitmap unchanged.
 */
SK_API bool SkCreateBitmapFromCGImage(SkBitmap* dst, CGImageRef src, SkISize* scaleToFit = NULL);

/**
 *  Copy the pixels from src into the memory specified by info/rowBytes/dstPixels. On failure,
 *  return false (e.g. ImageInfo incompatible with src).
 */
SK_API bool SkCopyPixelsFromCGImage(const SkImageInfo& info, size_t rowBytes, void* dstPixels,
                                    CGImageRef src);

/**
 *  Create an imageref from the specified bitmap using the specified colorspace.
 *  If space is NULL, then CGColorSpaceCreateDeviceRGB() is used.
 */
SK_API CGImageRef SkCreateCGImageRefWithColorspace(const SkBitmap& bm,
                                                   CGColorSpaceRef space);

/**
 *  Create an imageref from the specified bitmap using the colorspace returned
 *  by CGColorSpaceCreateDeviceRGB()
 */
static inline CGImageRef SkCreateCGImageRef(const SkBitmap& bm) {
    return SkCreateCGImageRefWithColorspace(bm, NULL);
}

/**
 *  Draw the bitmap into the specified CG context. The bitmap will be converted
 *  to a CGImage using the generic RGB colorspace. (x,y) specifies the position
 *  of the top-left corner of the bitmap. The bitmap is converted using the
 *  colorspace returned by CGColorSpaceCreateDeviceRGB()
 */
void SkCGDrawBitmap(CGContextRef, const SkBitmap&, float x, float y);

/**
 *  Create an SkBitmap drawing of the encoded PDF document, returning true on
 *  success. Deletes the stream when finished.
 */
bool SkPDFDocumentToBitmap(SkStream* stream, SkBitmap* output);

/**
 *  Return a provider that wraps the specified stream. It will become the only
 *  owner of the stream, so the caller must stop referring to the stream.
 *
 *  When the provider is finally deleted, it will delete the stream.
 */
CGDataProviderRef SkCreateDataProviderFromStream(SkStream*);

CGDataProviderRef SkCreateDataProviderFromData(SkData*);

#endif  // defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
#endif  // SkCGUtils_DEFINED
