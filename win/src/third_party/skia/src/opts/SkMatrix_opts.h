/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMatrix_opts_DEFINED
#define SkMatrix_opts_DEFINED

#include "include/core/SkMatrix.h"
#include "src/core/SkNx.h"

namespace SK_OPTS_NS {

static void matrix_translate(const SkMatrix& m, SkPoint* dst, const SkPoint* src, int count) {
    SkASSERT(m.getType() <= SkMatrix::kTranslate_Mask);
    if (count > 0) {
        SkScalar tx = m.getTranslateX();
        SkScalar ty = m.getTranslateY();
        if (count & 1) {
            dst->fX = src->fX + tx;
            dst->fY = src->fY + ty;
            src += 1;
            dst += 1;
        }
        Sk4s trans4(tx, ty, tx, ty);
        count >>= 1;
        if (count & 1) {
            (Sk4s::Load(&src->fX) + trans4).store(&dst->fX);
            src += 2;
            dst += 2;
        }
        count >>= 1;
        for (int i = 0; i < count; ++i) {
            (Sk4s::Load(&src[0].fX) + trans4).store(&dst[0].fX);
            (Sk4s::Load(&src[2].fX) + trans4).store(&dst[2].fX);
            src += 4;
            dst += 4;
        }
    }
}

static void matrix_scale_translate(const SkMatrix& m, SkPoint* dst, const SkPoint* src, int count) {
    SkASSERT(m.getType() <= (SkMatrix::kScale_Mask | SkMatrix::kTranslate_Mask));
    if (count > 0) {
        SkScalar tx = m.getTranslateX();
        SkScalar ty = m.getTranslateY();
        SkScalar sx = m.getScaleX();
        SkScalar sy = m.getScaleY();
        if (count & 1) {
            dst->fX = src->fX * sx + tx;
            dst->fY = src->fY * sy + ty;
            src += 1;
            dst += 1;
        }
        Sk4s trans4(tx, ty, tx, ty);
        Sk4s scale4(sx, sy, sx, sy);
        count >>= 1;
        if (count & 1) {
            (Sk4s::Load(&src->fX) * scale4 + trans4).store(&dst->fX);
            src += 2;
            dst += 2;
        }
        count >>= 1;
        for (int i = 0; i < count; ++i) {
            (Sk4s::Load(&src[0].fX) * scale4 + trans4).store(&dst[0].fX);
            (Sk4s::Load(&src[2].fX) * scale4 + trans4).store(&dst[2].fX);
            src += 4;
            dst += 4;
        }
    }
}

static void matrix_affine(const SkMatrix& m, SkPoint* dst, const SkPoint* src, int count) {
    SkASSERT(m.getType() != SkMatrix::kPerspective_Mask);
    if (count > 0) {
        SkScalar tx = m.getTranslateX();
        SkScalar ty = m.getTranslateY();
        SkScalar sx = m.getScaleX();
        SkScalar sy = m.getScaleY();
        SkScalar kx = m.getSkewX();
        SkScalar ky = m.getSkewY();
        if (count & 1) {
            dst->set(src->fX * sx + src->fY * kx + tx,
                     src->fX * ky + src->fY * sy + ty);
            src += 1;
            dst += 1;
        }
        Sk4s trans4(tx, ty, tx, ty);
        Sk4s scale4(sx, sy, sx, sy);
        Sk4s  skew4(kx, ky, kx, ky);    // applied to swizzle of src4
        count >>= 1;
        for (int i = 0; i < count; ++i) {
            Sk4s src4 = Sk4s::Load(&src->fX);
            Sk4s swz4(src[0].fY, src[0].fX, src[1].fY, src[1].fX);  // need ABCD -> BADC
            (src4 * scale4 + swz4 * skew4 + trans4).store(&dst->fX);
            src += 2;
            dst += 2;
        }
    }
}

} // namespace SK_OPTS_NS

#endif//SkMatrix_opts_DEFINED
