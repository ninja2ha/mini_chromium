
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkSnapShot_DEFINED
#define SkSnapShot_DEFINED

#include "src/animator/SkADrawable.h"
#include "include/core/SkImageDecoder.h"
#include "src/animator/SkMemberInfo.h"
#include "include/core/SkString.h"

class SkSnapshot: public SkADrawable {
    DECLARE_MEMBER_INFO(Snapshot);
    SkSnapshot();
    bool draw(SkAnimateMaker& ) override;
    private:
    SkString filename;
    SkScalar quality;
    SkBool sequence;
    int /*SkImageEncoder::Type*/    type;
    int fSeqVal;
};

#endif // SkSnapShot_DEFINED
