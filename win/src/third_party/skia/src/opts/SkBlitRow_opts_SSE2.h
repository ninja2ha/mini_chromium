/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlitRow_opts_SSE2_DEFINED
#define SkBlitRow_opts_SSE2_DEFINED

#include "include/core/SkBlitRow.h"

void S32_Blend_BlitRow32_SSE2(SkPMColor* SK_RESTRICT dst,
                              const SkPMColor* SK_RESTRICT src,
                              int count, U8CPU alpha);

void S32A_Opaque_BlitRow32_SSE2(SkPMColor* SK_RESTRICT dst,
                                const SkPMColor* SK_RESTRICT src,
                                int count, U8CPU alpha);

void S32A_Blend_BlitRow32_SSE2(SkPMColor* SK_RESTRICT dst,
                               const SkPMColor* SK_RESTRICT src,
                               int count, U8CPU alpha);

void Color32A_D565_SSE2(uint16_t dst[], SkPMColor src, int count, int x,
                        int y);

void SkBlitLCD16Row_SSE2(SkPMColor dst[], const uint16_t src[],
                         SkColor color, int width, SkPMColor);
void SkBlitLCD16OpaqueRow_SSE2(SkPMColor dst[], const uint16_t src[],
                               SkColor color, int width, SkPMColor opaqueDst);

void S32_D565_Opaque_SSE2(uint16_t* SK_RESTRICT dst,
                          const SkPMColor* SK_RESTRICT src, int count,
                          U8CPU alpha, int /*x*/, int /*y*/);
void S32A_D565_Opaque_SSE2(uint16_t* SK_RESTRICT dst,
                           const SkPMColor* SK_RESTRICT src,
                           int count, U8CPU alpha, int /*x*/, int /*y*/);
void S32_D565_Opaque_Dither_SSE2(uint16_t* SK_RESTRICT dst,
                                 const SkPMColor* SK_RESTRICT src,
                                 int count, U8CPU alpha, int x, int y);
void S32A_D565_Opaque_Dither_SSE2(uint16_t* SK_RESTRICT dst,
                                  const SkPMColor* SK_RESTRICT src,
                                  int count, U8CPU alpha, int x, int y);
#endif
