
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "src/core/SkSpriteBlitter.h"
#include "include/core/SkBlitRow.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorPriv.h"
#include "include/private/SkTemplates.h"
#include "include/core/SkUtils.h"
#include "include/core/SkXfermode.h"

///////////////////////////////////////////////////////////////////////////////

class Sprite_D32_S32 : public SkSpriteBlitter {
public:
    Sprite_D32_S32(const SkPixmap& src, U8CPU alpha)  : INHERITED(src) {
        SkASSERT(src.colorType() == kN32_SkColorType);

        unsigned flags32 = 0;
        if (255 != alpha) {
            flags32 |= SkBlitRow::kGlobalAlpha_Flag32;
        }
        if (!src.isOpaque()) {
            flags32 |= SkBlitRow::kSrcPixelAlpha_Flag32;
        }

        fProc32 = SkBlitRow::Factory32(flags32);
        fAlpha = alpha;
    }

    void blitRect(int x, int y, int width, int height) override {
        SkASSERT(width > 0 && height > 0);
        uint32_t* SK_RESTRICT dst = fDst.writable_addr32(x, y);
        const uint32_t* SK_RESTRICT src = fSource.addr32(x - fLeft, y - fTop);
        size_t dstRB = fDst.rowBytes();
        size_t srcRB = fSource.rowBytes();
        SkBlitRow::Proc32 proc = fProc32;
        U8CPU             alpha = fAlpha;

        do {
            proc(dst, src, width, alpha);
            dst = (uint32_t* SK_RESTRICT)((char*)dst + dstRB);
            src = (const uint32_t* SK_RESTRICT)((const char*)src + srcRB);
        } while (--height != 0);
    }

private:
    SkBlitRow::Proc32   fProc32;
    U8CPU               fAlpha;

    typedef SkSpriteBlitter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class Sprite_D32_XferFilter : public SkSpriteBlitter {
public:
    Sprite_D32_XferFilter(const SkPixmap& source, const SkPaint& paint) : SkSpriteBlitter(source) {
        fColorFilter = paint.getColorFilter();
        SkSafeRef(fColorFilter);

        fXfermode = paint.getXfermode();
        SkSafeRef(fXfermode);

        fBufferSize = 0;
        fBuffer = nullptr;

        unsigned flags32 = 0;
        if (255 != paint.getAlpha()) {
            flags32 |= SkBlitRow::kGlobalAlpha_Flag32;
        }
        if (!source.isOpaque()) {
            flags32 |= SkBlitRow::kSrcPixelAlpha_Flag32;
        }

        fProc32 = SkBlitRow::Factory32(flags32);
        fAlpha = paint.getAlpha();
    }

    virtual ~Sprite_D32_XferFilter() {
        delete[] fBuffer;
        SkSafeUnref(fXfermode);
        SkSafeUnref(fColorFilter);
    }

    void setup(const SkPixmap& dst, int left, int top, const SkPaint& paint) override {
        this->INHERITED::setup(dst, left, top, paint);

        int width = dst.width();
        if (width > fBufferSize) {
            fBufferSize = width;
            delete[] fBuffer;
            fBuffer = new SkPMColor[width];
        }
    }

protected:
    SkColorFilter*      fColorFilter;
    SkXfermode*         fXfermode;
    int                 fBufferSize;
    SkPMColor*          fBuffer;
    SkBlitRow::Proc32   fProc32;
    U8CPU               fAlpha;

private:
    typedef SkSpriteBlitter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class Sprite_D32_S32A_XferFilter : public Sprite_D32_XferFilter {
public:
    Sprite_D32_S32A_XferFilter(const SkPixmap& source, const SkPaint& paint)
        : Sprite_D32_XferFilter(source, paint) {}

    void blitRect(int x, int y, int width, int height) override {
        SkASSERT(width > 0 && height > 0);
        uint32_t* SK_RESTRICT dst = fDst.writable_addr32(x, y);
        const uint32_t* SK_RESTRICT src = fSource.addr32(x - fLeft, y - fTop);
        size_t dstRB = fDst.rowBytes();
        size_t srcRB = fSource.rowBytes();
        SkColorFilter* colorFilter = fColorFilter;
        SkXfermode* xfermode = fXfermode;

        do {
            const SkPMColor* tmp = src;

            if (colorFilter) {
                colorFilter->filterSpan(src, width, fBuffer);
                tmp = fBuffer;
            }

            if (xfermode) {
                xfermode->xfer32(dst, tmp, width, nullptr);
            } else {
                fProc32(dst, tmp, width, fAlpha);
            }

            dst = (uint32_t* SK_RESTRICT)((char*)dst + dstRB);
            src = (const uint32_t* SK_RESTRICT)((const char*)src + srcRB);
        } while (--height != 0);
    }

private:
    typedef Sprite_D32_XferFilter INHERITED;
};

static void fillbuffer(SkPMColor* SK_RESTRICT dst,
                       const SkPMColor16* SK_RESTRICT src, int count) {
    SkASSERT(count > 0);

    do {
        *dst++ = SkPixel4444ToPixel32(*src++);
    } while (--count != 0);
}

class Sprite_D32_S4444_XferFilter : public Sprite_D32_XferFilter {
public:
    Sprite_D32_S4444_XferFilter(const SkPixmap& source, const SkPaint& paint)
        : Sprite_D32_XferFilter(source, paint) {}

    void blitRect(int x, int y, int width, int height) override {
        SkASSERT(width > 0 && height > 0);
        SkPMColor* SK_RESTRICT dst = fDst.writable_addr32(x, y);
        const SkPMColor16* SK_RESTRICT src = fSource.addr16(x - fLeft, y - fTop);
        size_t dstRB = fDst.rowBytes();
        size_t srcRB = fSource.rowBytes();
        SkPMColor* SK_RESTRICT buffer = fBuffer;
        SkColorFilter* colorFilter = fColorFilter;
        SkXfermode* xfermode = fXfermode;

        do {
            fillbuffer(buffer, src, width);

            if (colorFilter) {
                colorFilter->filterSpan(buffer, width, buffer);
            }
            if (xfermode) {
                xfermode->xfer32(dst, buffer, width, nullptr);
            } else {
                fProc32(dst, buffer, width, fAlpha);
            }

            dst = (SkPMColor* SK_RESTRICT)((char*)dst + dstRB);
            src = (const SkPMColor16* SK_RESTRICT)((const char*)src + srcRB);
        } while (--height != 0);
    }

private:
    typedef Sprite_D32_XferFilter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static void src_row(SkPMColor* SK_RESTRICT dst,
                    const SkPMColor16* SK_RESTRICT src, int count) {
    do {
        *dst = SkPixel4444ToPixel32(*src);
        src += 1;
        dst += 1;
    } while (--count != 0);
}

class Sprite_D32_S4444_Opaque : public SkSpriteBlitter {
public:
    Sprite_D32_S4444_Opaque(const SkPixmap& source) : SkSpriteBlitter(source) {}

    void blitRect(int x, int y, int width, int height) override {
        SkASSERT(width > 0 && height > 0);
        SkPMColor* SK_RESTRICT dst = fDst.writable_addr32(x, y);
        const SkPMColor16* SK_RESTRICT src = fSource.addr16(x - fLeft, y - fTop);
        size_t dstRB = fDst.rowBytes();
        size_t srcRB = fSource.rowBytes();

        do {
            src_row(dst, src, width);
            dst = (SkPMColor* SK_RESTRICT)((char*)dst + dstRB);
            src = (const SkPMColor16* SK_RESTRICT)((const char*)src + srcRB);
        } while (--height != 0);
    }
};

static void srcover_row(SkPMColor* SK_RESTRICT dst,
                        const SkPMColor16* SK_RESTRICT src, int count) {
    do {
        *dst = SkPMSrcOver(SkPixel4444ToPixel32(*src), *dst);
        src += 1;
        dst += 1;
    } while (--count != 0);
}

class Sprite_D32_S4444 : public SkSpriteBlitter {
public:
    Sprite_D32_S4444(const SkPixmap& source) : SkSpriteBlitter(source) {}

    void blitRect(int x, int y, int width, int height) override {
        SkASSERT(width > 0 && height > 0);
        SkPMColor* SK_RESTRICT dst = fDst.writable_addr32(x, y);
        const SkPMColor16* SK_RESTRICT src = fSource.addr16(x - fLeft, y - fTop);
        size_t dstRB = fDst.rowBytes();
        size_t srcRB = fSource.rowBytes();

        do {
            srcover_row(dst, src, width);
            dst = (SkPMColor* SK_RESTRICT)((char*)dst + dstRB);
            src = (const SkPMColor16* SK_RESTRICT)((const char*)src + srcRB);
        } while (--height != 0);
    }
};

///////////////////////////////////////////////////////////////////////////////

SkSpriteBlitter* SkSpriteBlitter::ChooseD32(const SkPixmap& source, const SkPaint& paint,
        SkTBlitterAllocator* allocator) {
    SkASSERT(allocator != nullptr);

    if (paint.getMaskFilter() != nullptr) {
        return nullptr;
    }

    U8CPU       alpha = paint.getAlpha();
    SkXfermode* xfermode = paint.getXfermode();
    SkColorFilter* filter = paint.getColorFilter();
    SkSpriteBlitter* blitter = nullptr;

    switch (source.colorType()) {
        case kARGB_4444_SkColorType:
            if (alpha != 0xFF) {
                return nullptr;    // we only have opaque sprites
            }
            if (xfermode || filter) {
                blitter = allocator->createT<Sprite_D32_S4444_XferFilter>(source, paint);
            } else if (source.isOpaque()) {
                blitter = allocator->createT<Sprite_D32_S4444_Opaque>(source);
            } else {
                blitter = allocator->createT<Sprite_D32_S4444>(source);
            }
            break;
        case kN32_SkColorType:
            if (xfermode || filter) {
                if (255 == alpha) {
                    // this can handle xfermode or filter, but not alpha
                    blitter = allocator->createT<Sprite_D32_S32A_XferFilter>(source, paint);
                }
            } else {
                // this can handle alpha, but not xfermode or filter
                blitter = allocator->createT<Sprite_D32_S32>(source, alpha);
            }
            break;
        default:
            break;
    }
    return blitter;
}
