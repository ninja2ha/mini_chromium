
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkMatrixParts_DEFINED
#define SkMatrixParts_DEFINED

#include "src/animator/SkDisplayable.h"
#include "src/animator/SkMemberInfo.h"
#include "include/core/SkPathMeasure.h"

class SkDrawPath;
class SkDrawRect;
class SkPolygon;

class SkDrawMatrix;
// class SkMatrix;

class SkMatrixPart : public SkDisplayable {
public:
    SkMatrixPart();
    virtual bool add() = 0;
    virtual void dirty();
    virtual SkDisplayable* getParent() const;
    virtual bool setParent(SkDisplayable* parent);
#ifdef SK_DEBUG
    virtual bool isMatrixPart() const { return true; }
#endif
protected:
    SkDrawMatrix* fMatrix;
};

class SkRotate : public SkMatrixPart {
    DECLARE_MEMBER_INFO(Rotate);
    SkRotate();
protected:
    bool add() override;
    SkScalar degrees;
    SkPoint center;
};

class SkScale : public SkMatrixPart {
    DECLARE_MEMBER_INFO(Scale);
    SkScale();
protected:
    bool add() override;
    SkScalar x;
    SkScalar y;
    SkPoint center;
};

class SkSkew : public SkMatrixPart {
    DECLARE_MEMBER_INFO(Skew);
    SkSkew();
protected:
    bool add() override;
    SkScalar x;
    SkScalar y;
    SkPoint center;
};

class SkTranslate : public SkMatrixPart {
    DECLARE_MEMBER_INFO(Translate);
    SkTranslate();
protected:
    bool add() override;
    SkScalar x;
    SkScalar y;
};

class SkFromPath : public SkMatrixPart {
    DECLARE_MEMBER_INFO(FromPath);
    SkFromPath();
    virtual ~SkFromPath();
protected:
    bool add() override;
    int32_t mode;
    SkScalar offset;
    SkDrawPath* path;
    SkPathMeasure fPathMeasure;
};

class SkRectToRect : public SkMatrixPart {
    DECLARE_MEMBER_INFO(RectToRect);
    SkRectToRect();
    virtual ~SkRectToRect();
#ifdef SK_DUMP_ENABLED
    void dump(SkAnimateMaker* ) override;
#endif
    const SkMemberInfo* preferredChild(SkDisplayTypes type) override;
protected:
    bool add() override;
    SkDrawRect* source;
    SkDrawRect* destination;
};

class SkPolyToPoly : public SkMatrixPart {
    DECLARE_MEMBER_INFO(PolyToPoly);
    SkPolyToPoly();
    virtual ~SkPolyToPoly();
#ifdef SK_DUMP_ENABLED
    void dump(SkAnimateMaker* ) override;
#endif
    void onEndElement(SkAnimateMaker& ) override;
    const SkMemberInfo* preferredChild(SkDisplayTypes type) override;
protected:
    bool add() override;
    SkPolygon* source;
    SkPolygon* destination;
};

// !!! add concat matrix ?

#endif // SkMatrixParts_DEFINED
