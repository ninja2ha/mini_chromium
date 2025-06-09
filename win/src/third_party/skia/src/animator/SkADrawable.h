
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkADrawable_DEFINED
#define SkADrawable_DEFINED

#include "src/animator/SkDisplayable.h"
#include "src/animator/SkDisplayEvent.h"
#include "include/core/SkMath.h"

struct SkEventState;

class SkADrawable :  public SkDisplayable {
public:
    virtual bool doEvent(SkDisplayEvent::Kind , SkEventState* state );
    virtual bool draw(SkAnimateMaker& ) = 0;
    virtual void initialize();
    virtual bool isDrawable() const;
    virtual void setSteps(int steps);
};

#endif // SkADrawable_DEFINED
