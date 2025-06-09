/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/utils/SkBitmapSourceDeserializer.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkFilterQuality.h"
#include "include/core/SkImage.h"
#include "include/effects/SkImageSource.h"
#include "src/core/SkReadBuffer.h"

SkFlattenable* SkBitmapSourceDeserializer::CreateProc(SkReadBuffer& buffer) {
    SkFilterQuality filterQuality;
    if (buffer.isVersionLT(SkReadBuffer::kBitmapSourceFilterQuality_Version)) {
        filterQuality = kHigh_SkFilterQuality;
    } else {
        filterQuality = (SkFilterQuality)buffer.readInt();
    }
    SkRect src, dst;
    buffer.readRect(&src);
    buffer.readRect(&dst);
    SkBitmap bitmap;
    if (!buffer.readBitmap(&bitmap)) {
        return nullptr;
    }
    bitmap.setImmutable();

    SkAutoTUnref<SkImage> image(SkImage::NewFromBitmap(bitmap));
    return SkImageSource::Create(image, src, dst, filterQuality);
}
