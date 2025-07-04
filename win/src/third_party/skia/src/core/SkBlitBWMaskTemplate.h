
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "include/core/SkBitmap.h"
#include "include/core/SkMask.h"

#ifndef ClearLow3Bits_DEFINED
#define ClearLow3Bits_DEFINED
    #define ClearLow3Bits(x)    ((unsigned)(x) >> 3 << 3)
#endif

/*
    SK_BLITBWMASK_NAME          name of function(const SkBitmap& bitmap, const SkMask& mask, const SkIRect& clip, SK_BLITBWMASK_ARGS)
    SK_BLITBWMASK_ARGS          list of additional arguments to SK_BLITBWMASK_NAME, beginning with a comma
    SK_BLITBWMASK_BLIT8         name of function(U8CPU byteMask, SK_BLITBWMASK_DEVTYPE* dst, int x, int y)
    SK_BLITBWMASK_GETADDR       either writable_addr[8,16,32]
    SK_BLITBWMASK_DEVTYPE       either U32 or U16 or U8
*/

static void SK_BLITBWMASK_NAME(const SkPixmap& dst, const SkMask& srcMask,
                               const SkIRect& clip SK_BLITBWMASK_ARGS) {
    SkASSERT(clip.fRight <= srcMask.fBounds.fRight);

    int cx = clip.fLeft;
    int cy = clip.fTop;
    int maskLeft = srcMask.fBounds.fLeft;
    unsigned mask_rowBytes = srcMask.fRowBytes;
    size_t bitmap_rowBytes = dst.rowBytes();
    unsigned height = clip.height();

    SkASSERT(mask_rowBytes != 0);
    SkASSERT(bitmap_rowBytes != 0);
    SkASSERT(height != 0);

    const uint8_t* bits = srcMask.getAddr1(cx, cy);
    SK_BLITBWMASK_DEVTYPE* device = dst.SK_BLITBWMASK_GETADDR(cx, cy);

    if (cx == maskLeft && clip.fRight == srcMask.fBounds.fRight)
    {
        do {
            SK_BLITBWMASK_DEVTYPE* dst = device;
            unsigned rb = mask_rowBytes;
            do {
                U8CPU mask = *bits++;
                SK_BLITBWMASK_BLIT8(mask, dst);
                dst += 8;
            } while (--rb != 0);
            device = (SK_BLITBWMASK_DEVTYPE*)((char*)device + bitmap_rowBytes);
        } while (--height != 0);
    }
    else
    {
        int left_edge = cx - maskLeft;
        SkASSERT(left_edge >= 0);
        int rite_edge = clip.fRight - maskLeft;
        SkASSERT(rite_edge > left_edge);

        int left_mask = 0xFF >> (left_edge & 7);
        int rite_mask = 0xFF << (8 - (rite_edge & 7));
        rite_mask &= 0xFF;  // only want low-8 bits of mask
        int full_runs = (rite_edge >> 3) - ((left_edge + 7) >> 3);

        // check for empty right mask, so we don't read off the end (or go slower than we need to)
        if (rite_mask == 0)
        {
            SkASSERT(full_runs >= 0);
            full_runs -= 1;
            rite_mask = 0xFF;
        }
        if (left_mask == 0xFF)
            full_runs -= 1;

        // back up manually so we can keep in sync with our byte-aligned src
        // and not trigger an assert from the getAddr## function
        device -= left_edge & 7;

        if (full_runs < 0)
        {
            left_mask &= rite_mask;
            SkASSERT(left_mask != 0);
            do {
                U8CPU mask = *bits & left_mask;
                SK_BLITBWMASK_BLIT8(mask, device);
                bits += mask_rowBytes;
                device = (SK_BLITBWMASK_DEVTYPE*)((char*)device + bitmap_rowBytes);
            } while (--height != 0);
        }
        else
        {
            do {
                int runs = full_runs;
                SK_BLITBWMASK_DEVTYPE* dst = device;
                const uint8_t* b = bits;
                U8CPU   mask;

                mask = *b++ & left_mask;
                SK_BLITBWMASK_BLIT8(mask, dst);
                dst += 8;

                while (--runs >= 0)
                {
                    mask = *b++;
                    SK_BLITBWMASK_BLIT8(mask, dst);
                    dst += 8;
                }

                mask = *b & rite_mask;
                SK_BLITBWMASK_BLIT8(mask, dst);

                bits += mask_rowBytes;
                device = (SK_BLITBWMASK_DEVTYPE*)((char*)device + bitmap_rowBytes);
            } while (--height != 0);
        }
    }
}

#undef SK_BLITBWMASK_NAME
#undef SK_BLITBWMASK_ARGS
#undef SK_BLITBWMASK_BLIT8
#undef SK_BLITBWMASK_GETADDR
#undef SK_BLITBWMASK_DEVTYPE
#undef SK_BLITBWMASK_DOROWSETUP
