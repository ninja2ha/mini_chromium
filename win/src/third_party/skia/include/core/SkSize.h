/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSize_DEFINED
#define SkSize_DEFINED

#include "include/core/SkScalar.h"

template <typename T> struct SkTSize {
    T fWidth;
    T fHeight;

    static SkTSize Make(T w, T h) {
        SkTSize s;
        s.fWidth = w;
        s.fHeight = h;
        return s;
    }

    void set(T w, T h) {
        fWidth = w;
        fHeight = h;
    }

    /** Returns true iff fWidth == 0 && fHeight == 0
     */
    bool isZero() const {
        return 0 == fWidth && 0 == fHeight;
    }

    /** Returns true if either widht or height are <= 0 */
    bool isEmpty() const {
        return fWidth <= 0 || fHeight <= 0;
    }

    /** Set the width and height to 0 */
    void setEmpty() {
        fWidth = fHeight = 0;
    }

    T width() const { return fWidth; }
    T height() const { return fHeight; }

    /** If width or height is < 0, it is set to 0 */
    void clampNegToZero() {
        if (fWidth < 0) {
            fWidth = 0;
        }
        if (fHeight < 0) {
            fHeight = 0;
        }
    }

    bool equals(T w, T h) const {
        return fWidth == w && fHeight == h;
    }
};

template <typename T>
static inline bool operator==(const SkTSize<T>& a, const SkTSize<T>& b) {
    return a.fWidth == b.fWidth && a.fHeight == b.fHeight;
}

template <typename T>
static inline bool operator!=(const SkTSize<T>& a, const SkTSize<T>& b) {
    return !(a == b);
}

///////////////////////////////////////////////////////////////////////////////

typedef SkTSize<int32_t> SkISize;

struct SkSize : public SkTSize<SkScalar> {
    static SkSize Make(SkScalar w, SkScalar h) {
        SkSize s;
        s.fWidth = w;
        s.fHeight = h;
        return s;
    }


    SkSize& operator=(const SkISize& src) {
        this->set(SkIntToScalar(src.fWidth), SkIntToScalar(src.fHeight));
        return *this;
    }

    SkISize toRound() const {
        SkISize s;
        s.set(SkScalarRoundToInt(fWidth), SkScalarRoundToInt(fHeight));
        return s;
    }

    SkISize toCeil() const {
        SkISize s;
        s.set(SkScalarCeilToInt(fWidth), SkScalarCeilToInt(fHeight));
        return s;
    }

    SkISize toFloor() const {
        SkISize s;
        s.set(SkScalarFloorToInt(fWidth), SkScalarFloorToInt(fHeight));
        return s;
    }
};

#endif
