/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkFontDescriptor.h"
#include "include/core/SkStream.h"
#include "include/core/SkData.h"

enum {
    // these must match the sfnt 'name' enums
    kFontFamilyName = 0x01,
    kFullName       = 0x04,
    kPostscriptName = 0x06,

    // These count backwards from 0xFF, so as not to collide with the SFNT
    // defines for names in its 'name' table.
    kFontAxes       = 0xFC,
    kFontIndex      = 0xFD,
    kFontFileName   = 0xFE,  // Remove when MIN_PICTURE_VERSION > 41
    kSentinel       = 0xFF,
};

SkFontDescriptor::SkFontDescriptor(SkTypeface::Style style) : fStyle(style) { }

static void read_string(SkStream* stream, SkString* string) {
    const uint32_t length = SkToU32(stream->readPackedUInt());
    if (length > 0) {
        string->resize(length);
        stream->read(string->writable_str(), length);
    }
}

// Remove when MIN_PICTURE_VERSION > 41
static void skip_string(SkStream* stream) {
    const uint32_t length = SkToU32(stream->readPackedUInt());
    if (length > 0) {
        stream->skip(length);
    }
}

static void write_string(SkWStream* stream, const SkString& string, uint32_t id) {
    if (!string.isEmpty()) {
        stream->writePackedUInt(id);
        stream->writePackedUInt(string.size());
        stream->write(string.c_str(), string.size());
    }
}

static size_t read_uint(SkStream* stream) {
    return stream->readPackedUInt();
}

static void write_uint(SkWStream* stream, size_t n, uint32_t id) {
    stream->writePackedUInt(id);
    stream->writePackedUInt(n);
}

SkFontDescriptor::SkFontDescriptor(SkStream* stream) {
    fStyle = (SkTypeface::Style)stream->readPackedUInt();

    SkAutoSTMalloc<4, SkFixed> axis;
    size_t axisCount = 0;
    size_t index = 0;
    for (size_t id; (id = stream->readPackedUInt()) != kSentinel;) {
        switch (id) {
            case kFontFamilyName:
                read_string(stream, &fFamilyName);
                break;
            case kFullName:
                read_string(stream, &fFullName);
                break;
            case kPostscriptName:
                read_string(stream, &fPostscriptName);
                break;
            case kFontAxes:
                axisCount = read_uint(stream);
                axis.reset(axisCount);
                for (size_t i = 0; i < axisCount; ++i) {
                    axis[i] = read_uint(stream);
                }
                break;
            case kFontIndex:
                index = read_uint(stream);
                break;
            case kFontFileName:  // Remove when MIN_PICTURE_VERSION > 41
                skip_string(stream);
                break;
            default:
                SkDEBUGFAIL("Unknown id used by a font descriptor");
                return;
        }
    }

    size_t length = stream->readPackedUInt();
    if (length > 0) {
        SkAutoTUnref<SkData> data(SkData::NewUninitialized(length));
        if (stream->read(data->writable_data(), length) == length) {
            fFontData.reset(new SkFontData(new SkMemoryStream(data), index, axis, axisCount));
        }
    }
}

void SkFontDescriptor::serialize(SkWStream* stream) {
    stream->writePackedUInt(fStyle);

    write_string(stream, fFamilyName, kFontFamilyName);
    write_string(stream, fFullName, kFullName);
    write_string(stream, fPostscriptName, kPostscriptName);
    if (fFontData.get()) {
        if (fFontData->getIndex()) {
            write_uint(stream, fFontData->getIndex(), kFontIndex);
        }
        if (fFontData->getAxisCount()) {
            write_uint(stream, fFontData->getAxisCount(), kFontAxes);
            for (int i = 0; i < fFontData->getAxisCount(); ++i) {
                stream->writePackedUInt(fFontData->getAxis()[i]);
            }
        }
    }

    stream->writePackedUInt(kSentinel);

    if (fFontData.get() && fFontData->hasStream()) {
        SkAutoTDelete<SkStreamAsset> fontData(fFontData->detachStream());
        size_t length = fontData->getLength();
        stream->writePackedUInt(length);
        stream->writeStream(fontData, length);
    } else {
        stream->writePackedUInt(0);
    }
}
