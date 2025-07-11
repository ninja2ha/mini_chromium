
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "include/core/SkImageDecoder.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkMath.h"
#include "include/core/SkStream.h"
#include "include/private/SkTemplates.h"
#include "include/core/SkUtils.h"

class SkWBMPImageDecoder : public SkImageDecoder {
public:
    Format getFormat() const override {
        return kWBMP_Format;
    }

protected:
    Result onDecode(SkStream* stream, SkBitmap* bm, Mode) override;

private:
    typedef SkImageDecoder INHERITED;
};

static bool read_byte(SkStream* stream, uint8_t* data)
{
    return stream->read(data, 1) == 1;
}

static bool read_mbf(SkStream* stream, int* value)
{
    int n = 0;
    uint8_t data;
    do {
        if (!read_byte(stream, &data)) {
            return false;
        }
        n = (n << 7) | (data & 0x7F);
    } while (data & 0x80);

    *value = n;
    return true;
}

struct wbmp_head {
    int fWidth;
    int fHeight;

    bool init(SkStream* stream)
    {
        uint8_t data;

        if (!read_byte(stream, &data) || data != 0) { // unknown type
            return false;
        }
        if (!read_byte(stream, &data) || (data & 0x9F)) { // skip fixed header
            return false;
        }
        if (!read_mbf(stream, &fWidth) || (unsigned)fWidth > 0xFFFF) {
            return false;
        }
        if (!read_mbf(stream, &fHeight) || (unsigned)fHeight > 0xFFFF) {
            return false;
        }
        return fWidth != 0 && fHeight != 0;
    }
};

static void expand_bits_to_bytes(uint8_t dst[], const uint8_t src[], int bits)
{
    int bytes = bits >> 3;

    for (int i = 0; i < bytes; i++) {
        unsigned mask = *src++;
        dst[0] = (mask >> 7) & 1;
        dst[1] = (mask >> 6) & 1;
        dst[2] = (mask >> 5) & 1;
        dst[3] = (mask >> 4) & 1;
        dst[4] = (mask >> 3) & 1;
        dst[5] = (mask >> 2) & 1;
        dst[6] = (mask >> 1) & 1;
        dst[7] = (mask >> 0) & 1;
        dst += 8;
    }

    bits &= 7;
    if (bits > 0) {
        unsigned mask = *src;
        do {
            *dst++ = (mask >> 7) & 1;
            mask <<= 1;
        } while (--bits != 0);
    }
}

SkImageDecoder::Result SkWBMPImageDecoder::onDecode(SkStream* stream, SkBitmap* decodedBitmap,
                                                    Mode mode)
{
    wbmp_head   head;

    if (!head.init(stream)) {
        return kFailure;
    }

    int width = head.fWidth;
    int height = head.fHeight;

    decodedBitmap->setInfo(SkImageInfo::Make(width, height,
                                             kIndex_8_SkColorType, kOpaque_SkAlphaType));

    if (SkImageDecoder::kDecodeBounds_Mode == mode) {
        return kSuccess;
    }

    const SkPMColor colors[] = { SK_ColorBLACK, SK_ColorWHITE };
    SkColorTable* ct = new SkColorTable(colors, 2);
    SkAutoUnref   aur(ct);

    if (!this->allocPixelRef(decodedBitmap, ct)) {
        return kFailure;
    }

    SkAutoLockPixels alp(*decodedBitmap);

    uint8_t* dst = decodedBitmap->getAddr8(0, 0);
    // store the 1-bit valuess at the end of our pixels, so we won't stomp
    // on them before we're read them. Just trying to avoid a temp allocation
    size_t srcRB = SkAlign8(width) >> 3;
    size_t srcSize = height * srcRB;
    uint8_t* src = dst + decodedBitmap->getSize() - srcSize;
    if (stream->read(src, srcSize) != srcSize) {
        return kFailure;
    }

    for (int y = 0; y < height; y++)
    {
        expand_bits_to_bytes(dst, src, width);
        dst += decodedBitmap->rowBytes();
        src += srcRB;
    }

    return kSuccess;
}

///////////////////////////////////////////////////////////////////////////////
DEFINE_DECODER_CREATOR(WBMPImageDecoder);
///////////////////////////////////////////////////////////////////////////////

static SkImageDecoder* sk_wbmp_dfactory(SkStreamRewindable* stream) {
    wbmp_head   head;

    if (head.init(stream)) {
        return new SkWBMPImageDecoder;
    }
    return nullptr;
}

static SkImageDecoder::Format get_format_wbmp(SkStreamRewindable* stream) {
    wbmp_head head;
    if (head.init(stream)) {
        return SkImageDecoder::kWBMP_Format;
    }
    return SkImageDecoder::kUnknown_Format;
}

static SkImageDecoder_DecodeReg gDReg(sk_wbmp_dfactory);
static SkImageDecoder_FormatReg gFormatReg(get_format_wbmp);
