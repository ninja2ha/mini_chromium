/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkScalar_DEFINED
#define SkScalar_DEFINED

#include "include/core/SkFixed.h"
#include "include/private/SkFloatingPoint.h"

// TODO: move this sort of check into SkPostConfig.h
#define SK_SCALAR_IS_DOUBLE 0
#undef SK_SCALAR_IS_FLOAT
#define SK_SCALAR_IS_FLOAT  1


#if SK_SCALAR_IS_FLOAT

typedef float SkScalar;

#define SK_Scalar1                  1.0f
#define SK_ScalarHalf               0.5f
#define SK_ScalarSqrt2              1.41421356f
#define SK_ScalarPI                 3.14159265f
#define SK_ScalarTanPIOver8         0.414213562f
#define SK_ScalarRoot2Over2         0.707106781f
#define SK_ScalarMax                3.402823466e+38f
#define SK_ScalarInfinity           SK_FloatInfinity
#define SK_ScalarNegativeInfinity   SK_FloatNegativeInfinity
#define SK_ScalarNaN                SK_FloatNaN

#define SkFixedToScalar(x)          SkFixedToFloat(x)
#define SkScalarToFixed(x)          SkFloatToFixed(x)

#define SkScalarFloorToScalar(x)    sk_float_floor(x)
#define SkScalarCeilToScalar(x)     sk_float_ceil(x)
#define SkScalarRoundToScalar(x)    sk_float_floor((x) + 0.5f)

#define SkScalarFloorToInt(x)       sk_float_floor2int(x)
#define SkScalarCeilToInt(x)        sk_float_ceil2int(x)
#define SkScalarRoundToInt(x)       sk_float_round2int(x)

#define SkScalarAbs(x)              sk_float_abs(x)
#define SkScalarCopySign(x, y)      sk_float_copysign(x, y)
#define SkScalarMod(x, y)           sk_float_mod(x,y)
#define SkScalarFraction(x)         sk_float_mod(x, 1.0f)
#define SkScalarSqrt(x)             sk_float_sqrt(x)
#define SkScalarPow(b, e)           sk_float_pow(b, e)

#define SkScalarSin(radians)        (float)sk_float_sin(radians)
#define SkScalarCos(radians)        (float)sk_float_cos(radians)
#define SkScalarTan(radians)        (float)sk_float_tan(radians)
#define SkScalarASin(val)           (float)sk_float_asin(val)
#define SkScalarACos(val)           (float)sk_float_acos(val)
#define SkScalarATan2(y, x)         (float)sk_float_atan2(y,x)
#define SkScalarExp(x)              (float)sk_float_exp(x)
#define SkScalarLog(x)              (float)sk_float_log(x)
#define SkScalarLog2(x)             (float)sk_float_log2(x)

#else   // SK_SCALAR_IS_DOUBLE

typedef double SkScalar;

#define SK_Scalar1                  1.0
#define SK_ScalarHalf               0.5
#define SK_ScalarSqrt2              1.414213562373095
#define SK_ScalarPI                 3.141592653589793
#define SK_ScalarTanPIOver8         0.4142135623731
#define SK_ScalarRoot2Over2         0.70710678118655
#define SK_ScalarMax                1.7976931348623157+308
#define SK_ScalarInfinity           SK_DoubleInfinity
#define SK_ScalarNegativeInfinity   SK_DoubleNegativeInfinity
#define SK_ScalarNaN                SK_DoubleNaN

#define SkFixedToScalar(x)          SkFixedToDouble(x)
#define SkScalarToFixed(x)          SkDoubleToFixed(x)

#define SkScalarFloorToScalar(x)    floor(x)
#define SkScalarCeilToScalar(x)     ceil(x)
#define SkScalarRoundToScalar(x)    floor((x) + 0.5)

#define SkScalarFloorToInt(x)       (int)floor(x)
#define SkScalarCeilToInt(x)        (int)ceil(x)
#define SkScalarRoundToInt(x)       (int)floor((x) + 0.5)

#define SkScalarAbs(x)              abs(x)
#define SkScalarCopySign(x, y)      copysign(x, y)
#define SkScalarMod(x, y)           fmod(x,y)
#define SkScalarFraction(x)         fmod(x, 1.0)
#define SkScalarSqrt(x)             sqrt(x)
#define SkScalarPow(b, e)           pow(b, e)

#define SkScalarSin(radians)        sin(radians)
#define SkScalarCos(radians)        cos(radians)
#define SkScalarTan(radians)        tan(radians)
#define SkScalarASin(val)           asin(val)
#define SkScalarACos(val)           acos(val)
#define SkScalarATan2(y, x)         atan2(y,x)
#define SkScalarExp(x)              exp(x)
#define SkScalarLog(x)              log(x)
#define SkScalarLog2(x)             log2(x)

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////

#define SkIntToScalar(x)        static_cast<SkScalar>(x)
#define SkScalarTruncToInt(x)   static_cast<int>(x)

#define SkScalarToFloat(x)      static_cast<float>(x)
#define SkFloatToScalar(x)      static_cast<SkScalar>(x)
#define SkScalarToDouble(x)     static_cast<double>(x)
#define SkDoubleToScalar(x)     static_cast<SkScalar>(x)

#define SK_ScalarMin            (-SK_ScalarMax)

static inline bool SkScalarIsNaN(SkScalar x) { return x != x; }

/** Returns true if x is not NaN and not infinite
 */
static inline bool SkScalarIsFinite(SkScalar x) {
    // We rely on the following behavior of infinities and nans
    // 0 * finite --> 0
    // 0 * infinity --> NaN
    // 0 * NaN --> NaN
    SkScalar prod = x * 0;
    // At this point, prod will either be NaN or 0
    return !SkScalarIsNaN(prod);
}

static inline bool SkScalarsAreFinite(SkScalar a, SkScalar b) {
    SkScalar prod = 0;
    prod *= a;
    prod *= b;
    // At this point, prod will either be NaN or 0
    return !SkScalarIsNaN(prod);
}

static inline bool SkScalarsAreFinite(const SkScalar array[], int count) {
    SkScalar prod = 0;
    for (int i = 0; i < count; ++i) {
        prod *= array[i];
    }
    // At this point, prod will either be NaN or 0
    return !SkScalarIsNaN(prod);
}

/**
 *  Variant of SkScalarRoundToInt, that performs the rounding step (adding 0.5) explicitly using
 *  double, to avoid possibly losing the low bit(s) of the answer before calling floor().
 *
 *  This routine will likely be slower than SkScalarRoundToInt(), and should only be used when the
 *  extra precision is known to be valuable.
 *
 *  In particular, this catches the following case:
 *      SkScalar x = 0.49999997;
 *      int ix = SkScalarRoundToInt(x);
 *      SkASSERT(0 == ix);    // <--- fails
 *      ix = SkDScalarRoundToInt(x);
 *      SkASSERT(0 == ix);    // <--- succeeds
 */
static inline int SkDScalarRoundToInt(SkScalar x) {
    double xx = x;
    xx += 0.5;
    return (int)floor(xx);
}

static inline SkScalar SkScalarClampMax(SkScalar x, SkScalar max) {
    x = SkTMin(x, max);
    x = SkTMax<SkScalar>(x, 0);
    return x;
}

static inline SkScalar SkScalarPin(SkScalar x, SkScalar min, SkScalar max) {
    return SkTPin(x, min, max);
}

SkScalar SkScalarSinCos(SkScalar radians, SkScalar* cosValue);

static inline SkScalar SkScalarSquare(SkScalar x) { return x * x; }

#define SkScalarMul(a, b)       ((SkScalar)(a) * (b))
#define SkScalarMulAdd(a, b, c) ((SkScalar)(a) * (b) + (c))
#define SkScalarMulDiv(a, b, c) ((SkScalar)(a) * (b) / (c))
#define SkScalarInvert(x)       (SK_Scalar1 / (x))
#define SkScalarFastInvert(x)   (SK_Scalar1 / (x))
#define SkScalarAve(a, b)       (((a) + (b)) * SK_ScalarHalf)
#define SkScalarHalf(a)         ((a) * SK_ScalarHalf)

#define SkDegreesToRadians(degrees) ((degrees) * (SK_ScalarPI / 180))
#define SkRadiansToDegrees(radians) ((radians) * (180 / SK_ScalarPI))

static inline SkScalar SkMaxScalar(SkScalar a, SkScalar b) { return a > b ? a : b; }
static inline SkScalar SkMinScalar(SkScalar a, SkScalar b) { return a < b ? a : b; }

static inline bool SkScalarIsInt(SkScalar x) {
    return x == (SkScalar)(int)x;
}

/**
 *  Returns -1 || 0 || 1 depending on the sign of value:
 *  -1 if x < 0
 *   0 if x == 0
 *   1 if x > 0
 */
static inline int SkScalarSignAsInt(SkScalar x) {
    return x < 0 ? -1 : (x > 0);
}

// Scalar result version of above
static inline SkScalar SkScalarSignAsScalar(SkScalar x) {
    return x < 0 ? -SK_Scalar1 : ((x > 0) ? SK_Scalar1 : 0);
}

#define SK_ScalarNearlyZero         (SK_Scalar1 / (1 << 12))

static inline bool SkScalarNearlyZero(SkScalar x,
                                    SkScalar tolerance = SK_ScalarNearlyZero) {
    SkASSERT(tolerance >= 0);
    return SkScalarAbs(x) <= tolerance;
}

static inline bool SkScalarNearlyEqual(SkScalar x, SkScalar y,
                                       SkScalar tolerance = SK_ScalarNearlyZero) {
    SkASSERT(tolerance >= 0);
    return SkScalarAbs(x-y) <= tolerance;
}

/** Linearly interpolate between A and B, based on t.
    If t is 0, return A
    If t is 1, return B
    else interpolate.
    t must be [0..SK_Scalar1]
*/
static inline SkScalar SkScalarInterp(SkScalar A, SkScalar B, SkScalar t) {
    SkASSERT(t >= 0 && t <= SK_Scalar1);
    return A + (B - A) * t;
}

/** Interpolate along the function described by (keys[length], values[length])
    for the passed searchKey.  SearchKeys outside the range keys[0]-keys[Length]
    clamp to the min or max value.  This function was inspired by a desire
    to change the multiplier for thickness in fakeBold; therefore it assumes
    the number of pairs (length) will be small, and a linear search is used.
    Repeated keys are allowed for discontinuous functions (so long as keys is
    monotonically increasing), and if key is the value of a repeated scalar in
    keys, the first one will be used.  However, that may change if a binary
    search is used.
*/
SkScalar SkScalarInterpFunc(SkScalar searchKey, const SkScalar keys[],
                            const SkScalar values[], int length);

/*
 *  Helper to compare an array of scalars.
 */
static inline bool SkScalarsEqual(const SkScalar a[], const SkScalar b[], int n) {
    SkASSERT(n >= 0);
    for (int i = 0; i < n; ++i) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

#endif
