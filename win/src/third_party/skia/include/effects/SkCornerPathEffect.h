/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCornerPathEffect_DEFINED
#define SkCornerPathEffect_DEFINED

#include "include/core/SkPathEffect.h"

/** \class SkCornerPathEffect

    SkCornerPathEffect is a subclass of SkPathEffect that can turn sharp corners
    into various treatments (e.g. rounded corners)
*/
class SK_API SkCornerPathEffect : public SkPathEffect {
public:
    /** radius must be > 0 to have an effect. It specifies the distance from each corner
        that should be "rounded".
    */
    static SkPathEffect* Create(SkScalar radius) { return new SkCornerPathEffect(radius); }

    virtual bool filterPath(SkPath* dst, const SkPath& src,
                            SkStrokeRec*, const SkRect*) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkCornerPathEffect)

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    bool exposedInAndroidJavaAPI() const override { return true; }
#endif

protected:
    virtual ~SkCornerPathEffect();

    explicit SkCornerPathEffect(SkScalar radius);
    void flatten(SkWriteBuffer&) const override;

private:
    SkScalar    fRadius;

    typedef SkPathEffect INHERITED;
};

#endif
