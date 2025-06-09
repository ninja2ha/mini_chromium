
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDrawClip_DEFINED
#define SkDrawClip_DEFINED

#include "src/animator/SkADrawable.h"
#include "src/animator/SkMemberInfo.h"
#include "include/core/SkRegion.h"

class SkDrawPath;
class SkDrawRect;

class SkDrawClip : public SkADrawable {
    DECLARE_DRAW_MEMBER_INFO(Clip);
    SkDrawClip();
    bool draw(SkAnimateMaker& ) override;
private:
    SkDrawRect* rect;
    SkDrawPath* path;
};

#endif // SkDrawClip_DEFINED
