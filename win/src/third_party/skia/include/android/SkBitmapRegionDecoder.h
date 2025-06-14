/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapRegionDecoder_DEFINED
#define SkBitmapRegionDecoder_DEFINED

#include "include/core/SkBitmap.h"
#include "include/android/SkBRDAllocator.h"
#include "include/codec/SkEncodedFormat.h"
#include "include/core/SkStream.h"

/*
 * This class aims to provide an interface to test multiple implementations of
 * SkBitmapRegionDecoder.
 */
class SkBitmapRegionDecoder {
public:

    enum Strategy {
        kCanvas_Strategy,       // Draw to the canvas, uses SkCodec
        kAndroidCodec_Strategy, // Uses SkAndroidCodec for scaling and subsetting
    };

    /*
     * @param data     Refs the data while this object exists, unrefs on destruction
     * @param strategy Strategy used for scaling and subsetting
     * @return         Tries to create an SkBitmapRegionDecoder, returns NULL on failure
     */
    static SkBitmapRegionDecoder* Create(
            SkData* data, Strategy strategy);

    /*
     * @param stream   Takes ownership of the stream
     * @param strategy Strategy used for scaling and subsetting
     * @return         Tries to create an SkBitmapRegionDecoder, returns NULL on failure
     */
    static SkBitmapRegionDecoder* Create(
            SkStreamRewindable* stream, Strategy strategy);

    /*
     * Decode a scaled region of the encoded image stream
     *
     * @param bitmap          Container for decoded pixels.  It is assumed that the pixels
     *                        are initially unallocated and will be allocated by this function.
     * @param allocator       Allocator for the pixels.  If this is NULL, the default
     *                        allocator (HeapAllocator) will be used.
     * @param desiredSubset   Subset of the original image to decode.
     * @param sampleSize      An integer downscaling factor for the decode.
     * @param colorType       Preferred output colorType.
     *                        New implementations should return NULL if they do not support
     *                        decoding to this color type.
     *                        The old kOriginal_Strategy will decode to a default color type
     *                        if this color type is unsupported.
     * @param requireUnpremul If the image is not opaque, we will use this to determine the
     *                        alpha type to use.
     *
     */
    virtual bool decodeRegion(SkBitmap* bitmap, SkBRDAllocator* allocator,
                              const SkIRect& desiredSubset, int sampleSize,
                              SkColorType colorType, bool requireUnpremul) = 0;
    /*
     * @param  Requested destination color type
     * @return true if we support the requested color type and false otherwise
     */
    virtual bool conversionSupported(SkColorType colorType) = 0;

    virtual SkEncodedFormat getEncodedFormat() = 0;

    int width() const { return fWidth; }
    int height() const { return fHeight; }

    virtual ~SkBitmapRegionDecoder() {}

protected:

    SkBitmapRegionDecoder(int width, int height)
        : fWidth(width)
        , fHeight(height)
    {}

private:
    const int fWidth;
    const int fHeight;
};

#endif
