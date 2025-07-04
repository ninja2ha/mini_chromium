/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDisplayList_DEFINED
#define SkDisplayList_DEFINED

#include "src/animator/SkOperand.h"
#include "src/animator/SkIntArray.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"

class SkAnimateMaker;
class SkActive;
class SkApply;
class SkADrawable;
class SkGroup;

class SkDisplayList : public SkRefCnt {
public:
    SkDisplayList();
    virtual ~SkDisplayList();
    void append(SkActive* );
    void clear() { fDrawList.reset(); }
    int count() { return fDrawList.count(); }
    bool draw(SkAnimateMaker& , SkMSec time);
#ifdef SK_DUMP_ENABLED
    void dump(SkAnimateMaker* maker);
    void dumpInner(SkAnimateMaker* maker);
    static int fIndent;
    static int fDumpIndex;
#endif
    int findGroup(SkADrawable* match, SkTDDrawableArray** list,
        SkGroup** parent, SkGroup** found, SkTDDrawableArray** grandList);
    SkADrawable* get(int index) { return fDrawList[index]; }
    SkMSec getTime() { return fInTime; }
    SkTDDrawableArray* getDrawList() { return &fDrawList; }
    void hardReset();
    virtual bool onIRect(const SkIRect& r);
    void reset();
    void remove(SkActive* );
#ifdef SK_DEBUG
    void validate();
#else
    void validate() {}
#endif
    static int SearchForMatch(SkADrawable* match, SkTDDrawableArray** list,
        SkGroup** parent, SkGroup** found, SkTDDrawableArray**grandList);
    static bool SearchGroupForMatch(SkADrawable* draw, SkADrawable* match,
        SkTDDrawableArray** list, SkGroup** parent, SkGroup** found, SkTDDrawableArray** grandList,
        int &index);
public:
    SkIRect fBounds;
    SkIRect fInvalBounds;
    bool fDrawBounds;
    bool fHasUnion;
    bool fUnionBounds;
private:
    SkTDDrawableArray fDrawList;
    SkTDActiveArray fActiveList;
    SkMSec fInTime;
    friend class SkEvents;
};

#endif // SkDisplayList_DEFINED
