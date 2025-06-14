/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapProcState_DEFINED
#define SkBitmapProcState_DEFINED

#include "include/core/SkBitmap.h"
#include "src/core/SkBitmapController.h"
#include "src/core/SkBitmapFilter.h"
#include "src/core/SkBitmapProvider.h"
#include "include/private/SkFloatBits.h"
#include "include/core/SkMatrix.h"
#include "src/core/SkMipMap.h"
#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"
#include "include/private/SkTemplates.h"

typedef SkFixed3232    SkFractionalInt;
#define SkScalarToFractionalInt(x)  SkScalarToFixed3232(x)
#define SkFractionalIntToFixed(x)   SkFixed3232ToFixed(x)
#define SkFixedToFractionalInt(x)   SkFixedToFixed3232(x)
#define SkFractionalIntToInt(x)     SkFixed3232ToInt(x)

class SkPaint;

struct SkBitmapProcState {
    SkBitmapProcState(const SkBitmapProvider&, SkShader::TileMode tmx, SkShader::TileMode tmy);
    SkBitmapProcState(const SkBitmap&, SkShader::TileMode tmx, SkShader::TileMode tmy);
    ~SkBitmapProcState();

    typedef void (*ShaderProc32)(const void* ctx, int x, int y, SkPMColor[], int count);

    typedef void (*ShaderProc16)(const void* ctx, int x, int y, uint16_t[], int count);

    typedef void (*MatrixProc)(const SkBitmapProcState&,
                               uint32_t bitmapXY[],
                               int count,
                               int x, int y);

    typedef void (*SampleProc32)(const SkBitmapProcState&,
                                 const uint32_t[],
                                 int count,
                                 SkPMColor colors[]);

    typedef U16CPU (*FixedTileProc)(SkFixed);   // returns 0..0xFFFF
    typedef U16CPU (*FixedTileLowBitsProc)(SkFixed, int);   // returns 0..0xF
    typedef U16CPU (*IntTileProc)(int value, int count);   // returns 0..count-1

    SkPixmap            fPixmap;
    SkMatrix            fInvMatrix;         // copy of what is in fBMState, can we remove the dup?

    SkMatrix::MapXYProc fInvProc;           // chooseProcs

    SkFractionalInt     fInvSxFractionalInt;
    SkFractionalInt     fInvKyFractionalInt;

    FixedTileProc       fTileProcX;         // chooseProcs
    FixedTileProc       fTileProcY;         // chooseProcs
    FixedTileLowBitsProc fTileLowBitsProcX; // chooseProcs
    FixedTileLowBitsProc fTileLowBitsProcY; // chooseProcs
    IntTileProc         fIntTileProcY;      // chooseProcs
    SkFixed             fFilterOneX;
    SkFixed             fFilterOneY;

    SkPMColor           fPaintPMColor;      // chooseProcs - A8 config
    SkFixed             fInvSx;             // chooseProcs
    SkFixed             fInvKy;             // chooseProcs
    uint16_t            fAlphaScale;        // chooseProcs
    uint8_t             fInvType;           // chooseProcs
    uint8_t             fTileModeX;         // CONSTRUCTOR
    uint8_t             fTileModeY;         // CONSTRUCTOR
    uint8_t             fFilterLevel;       // chooseProcs

    /** Platforms implement this, and can optionally overwrite only the
        following fields:

        fShaderProc32
        fShaderProc16
        fMatrixProc
        fSampleProc32
        fSampleProc32

        They will already have valid function pointers, so a platform that does
        not have an accelerated version can just leave that field as is. A valid
        implementation can do nothing (see SkBitmapProcState_opts_none.cpp)
     */
    void platformProcs();

    /** Given the byte size of the index buffer to be passed to the matrix proc,
        return the maximum number of resulting pixels that can be computed
        (i.e. the number of SkPMColor values to be written by the sample proc).
        This routine takes into account that filtering and scale-vs-affine
        affect the amount of buffer space needed.

        Only valid to call after chooseProcs (setContext) has been called. It is
        safe to call this inside the shader's shadeSpan() method.
     */
    int maxCountForBufferSize(size_t bufferSize) const;

    // If a shader proc is present, then the corresponding matrix/sample procs
    // are ignored
    ShaderProc32 getShaderProc32() const { return fShaderProc32; }
    ShaderProc16 getShaderProc16() const { return fShaderProc16; }

#ifdef SK_DEBUG
    MatrixProc getMatrixProc() const;
#else
    MatrixProc getMatrixProc() const { return fMatrixProc; }
#endif
    SampleProc32 getSampleProc32() const { return fSampleProc32; }

private:
    friend class SkBitmapProcShader;
    friend class SkLightingShaderImpl;

    ShaderProc32        fShaderProc32;      // chooseProcs
    ShaderProc16        fShaderProc16;      // chooseProcs
    // These are used if the shaderproc is nullptr
    MatrixProc          fMatrixProc;        // chooseProcs
    SampleProc32        fSampleProc32;      // chooseProcs

    const SkBitmapProvider fProvider;

    enum {
        kBMStateSize = 136  // found by inspection. if too small, we will call new/delete
    };
    SkAlignedSStorage<kBMStateSize> fBMStateStorage;
    SkBitmapController::State* fBMState;

    MatrixProc chooseMatrixProc(bool trivial_matrix);
    bool chooseProcs(const SkMatrix& inv, const SkPaint&);
    bool chooseScanlineProcs(bool trivialMatrix, bool clampClamp, const SkPaint& paint);
    ShaderProc32 chooseShaderProc32();

    // Return false if we failed to setup for fast translate (e.g. overflow)
    bool setupForTranslate();

#ifdef SK_DEBUG
    static void DebugMatrixProc(const SkBitmapProcState&,
                                uint32_t[], int count, int x, int y);
#endif
};

/*  Macros for packing and unpacking pairs of 16bit values in a 32bit uint.
    Used to allow access to a stream of uint16_t either one at a time, or
    2 at a time by unpacking a uint32_t
 */
#ifdef SK_CPU_BENDIAN
    #define PACK_TWO_SHORTS(pri, sec) ((pri) << 16 | (sec))
    #define UNPACK_PRIMARY_SHORT(packed)    ((uint32_t)(packed) >> 16)
    #define UNPACK_SECONDARY_SHORT(packed)  ((packed) & 0xFFFF)
#else
    #define PACK_TWO_SHORTS(pri, sec) ((pri) | ((sec) << 16))
    #define UNPACK_PRIMARY_SHORT(packed)    ((packed) & 0xFFFF)
    #define UNPACK_SECONDARY_SHORT(packed)  ((uint32_t)(packed) >> 16)
#endif

#ifdef SK_DEBUG
    static inline uint32_t pack_two_shorts(U16CPU pri, U16CPU sec) {
        SkASSERT((uint16_t)pri == pri);
        SkASSERT((uint16_t)sec == sec);
        return PACK_TWO_SHORTS(pri, sec);
    }
#else
    #define pack_two_shorts(pri, sec)   PACK_TWO_SHORTS(pri, sec)
#endif

// These functions are generated via macros, but are exposed here so that
// platformProcs may test for them by name.
void S32_opaque_D32_filter_DX(const SkBitmapProcState& s, const uint32_t xy[],
                              int count, SkPMColor colors[]);
void S32_alpha_D32_filter_DX(const SkBitmapProcState& s, const uint32_t xy[],
                             int count, SkPMColor colors[]);
void S32_opaque_D32_filter_DXDY(const SkBitmapProcState& s,
                                const uint32_t xy[], int count, SkPMColor colors[]);
void S32_alpha_D32_filter_DXDY(const SkBitmapProcState& s,
                               const uint32_t xy[], int count, SkPMColor colors[]);
void ClampX_ClampY_filter_scale(const SkBitmapProcState& s, uint32_t xy[],
                                int count, int x, int y);
void ClampX_ClampY_nofilter_scale(const SkBitmapProcState& s, uint32_t xy[],
                                  int count, int x, int y);
void ClampX_ClampY_filter_affine(const SkBitmapProcState& s,
                                 uint32_t xy[], int count, int x, int y);
void ClampX_ClampY_nofilter_affine(const SkBitmapProcState& s,
                                   uint32_t xy[], int count, int x, int y);

// Helper class for mapping the middle of pixel (x, y) into SkFractionalInt bitmap space.
// TODO: filtered version which applies a fFilterOne{X,Y}/2 bias instead of epsilon?
class SkBitmapProcStateAutoMapper {
public:
    SkBitmapProcStateAutoMapper(const SkBitmapProcState& s, int x, int y) {
        SkPoint pt;
        s.fInvProc(s.fInvMatrix,
                   SkIntToScalar(x) + SK_ScalarHalf,
                   SkIntToScalar(y) + SK_ScalarHalf, &pt);

        // SkFixed epsilon bias to ensure inverse-mapped bitmap coordinates are rounded
        // consistently WRT geometry.  Note that we only need the bias for positive scales:
        // for negative scales, the rounding is intrinsically correct.
        // We scale it to persist SkFractionalInt -> SkFixed conversions.
        const SkFixed biasX = (s.fInvMatrix.getScaleX() > 0);
        const SkFixed biasY = (s.fInvMatrix.getScaleY() > 0);
        fX = SkScalarToFractionalInt(pt.x()) - SkFixedToFractionalInt(biasX);
        fY = SkScalarToFractionalInt(pt.y()) - SkFixedToFractionalInt(biasY);
    }

    SkFractionalInt x() const { return fX; }
    SkFractionalInt y() const { return fY; }

private:
    SkFractionalInt fX, fY;
};

#endif
