
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkAnimateMaker_DEFINED
#define SkAnimateMaker_DEFINED

// #define SK_DEBUG_ANIMATION_TIMING

#include "include/animator/SkAnimator.h"
#include "include/core/SkBitmap.h"
#include "src/animator/SkIntArray.h"
#include "src/animator/SkDisplayEvents.h"
#include "src/animator/SkDisplayList.h"
#include "src/animator/SkDisplayScreenplay.h"
#include "src/animator/SkDisplayXMLParser.h"
#include "src/animator/SkScript.h"
#include "include/core/SkString.h"
#include "include/private/SkTDict.h"

// not sure where this little helper macro should go


class SkActive;
class SkAnimate;
class SkCanvas;
class SkDisplayable;
class SkADrawable;
class SkDump;
class SkEvent;
class SkEventSink;
class SkExtras;
class SkGroup;
class SkPaint;
class SkStream;

class SkAnimateMaker {
public:
    SkAnimateMaker(SkAnimator* animator, SkCanvas* canvas, SkPaint* paint);
    ~SkAnimateMaker();
    void appendActive(SkActive* );
    void childrenAdd(SkDisplayable* child) { *fChildren.append() = child; }
    void clearExtraPropertyCallBack(SkDisplayTypes type);
    bool computeID(SkDisplayable* displayable, SkDisplayable* parent, SkString* newID);
    SkDisplayable* createInstance(const char name[], size_t len);
    bool decodeStream(SkStream* stream);
    bool decodeURI(const char uri[]);
    void delayEnable(SkApply* apply, SkMSec time);
    void doDelayedEvent();
    bool doEvent(const SkEvent& event);
#ifdef SK_DUMP_ENABLED
    void dump(const char* match);
#endif
    int dynamicProperty(SkString& nameStr, SkDisplayable**  );
    bool find(const char* str, SkDisplayable** displayablePtr) const {
        return fIDs.find(str, displayablePtr);
    }
    bool find(const char* str, size_t len, SkDisplayable** displayablePtr) const {
        return fIDs.find(str, len, displayablePtr);
    }
    bool findKey(SkDisplayable* displayable, const char** string) const {
        return fIDs.findKey(displayable, string);
    }
//  bool find(SkString& string, SkDisplayable** displayablePtr) {
//      return fIDs.find(string.c_str(), displayablePtr);
//  }
    SkAnimator* getAnimator() { return fAnimator; }
    SkMSec getAppTime() const; // call caller to get current time
#ifdef SK_DEBUG
    SkAnimator* getRoot();
#endif
    SkXMLParserError::ErrorCode getErrorCode() const { return fError.getErrorCode(); }
    SkMSec getInTime() { return fDisplayList.getTime(); }
    int getNativeCode() const { return fError.getNativeCode(); }
    bool hasError() { return fError.hasError(); }
    void helperAdd(SkDisplayable* trackMe);
    void helperRemove(SkDisplayable* alreadyTracked);
    void idsSet(const char* attrValue, size_t len, SkDisplayable* displayable) {
        fIDs.set(attrValue, len, displayable); }
//  void loadMovies();
    void notifyInval();
    void notifyInvalTime(SkMSec time);
    void postOnEnd(SkAnimateBase* animate, SkMSec end);
    void removeActive(SkActive* );
    void reset();
    bool resolveID(SkDisplayable* displayable, SkDisplayable* original);
    void setEnableTime(SkMSec appTime, SkMSec expectedTime);
    void setErrorCode(SkXMLParserError::ErrorCode err) { if (fError.hasError() == false) fError.INHERITED::setCode(err); }
    void setErrorCode(SkDisplayXMLParserError::ErrorCode err) { if (fError.hasError() == false) fError.setCode(err); }
    void setErrorNoun(const SkString& str) { if (fError.hasError() == false) fError.setNoun(str); }
    void setErrorString();
    void setExtraPropertyCallBack(SkDisplayTypes type, SkScriptEngine::_propertyCallBack , void* userStorage);
    void setID(SkDisplayable* displayable, const SkString& newID);
    void setInnerError(SkAnimateMaker* maker, const SkString& str) { fError.setInnerError(maker, str); }
    void setScriptError(const SkScriptEngine& );
#ifdef SK_DEBUG
    void validate() { fDisplayList.validate(); }
#else
    void validate() {}
#endif
    SkDisplayEvent* fActiveEvent;
    SkMSec fAdjustedStart;
    SkCanvas* fCanvas;
    SkMSec fEnableTime;
    int fEndDepth;  // passed parameter to onEndElement
    SkEvents fEvents;
    SkDisplayList fDisplayList;
    SkEventSinkID fHostEventSinkID;
    SkMSec fMinimumInterval;
    SkPaint* fPaint;
    SkAnimateMaker* fParentMaker;
    SkString fPrefix;
    SkDisplayScreenplay fScreenplay;
    const SkAnimator::Timeline* fTimeline;
    SkBool8 fInInclude;
    SkBool8 fInMovie;
    SkBool8 fFirstScriptError;
#if defined SK_DEBUG && defined SK_DEBUG_ANIMATION_TIMING
    SkMSec fDebugTimeBase;
#endif
#ifdef SK_DUMP_ENABLED
    SkString fDumpAnimated;
    SkBool8 fDumpEvents;
    SkBool8 fDumpGConditions;
    SkBool8 fDumpPosts;
#endif
private:
    void deleteMembers();
    static bool GetStep(const char* token, size_t len, void* stepPtr, SkScriptValue* );
    SkAnimateMaker& operator=(SkAnimateMaker& );
    SkTDDisplayableArray fChildren;
    SkTDDisplayableArray fDelayed; // SkApply that contain delayed enable events
    SkDisplayXMLParserError fError;
    SkString fErrorString;
    SkTDArray<SkExtras*> fExtras;
    SkString fFileName;
    SkTDDisplayableArray fHelpers;  // helper displayables
    SkBool8 fLoaded;
    SkTDDisplayableArray fMovies;
    SkTDict<SkDisplayable*> fIDs;
    SkAnimator* fAnimator;
    friend class SkAdd;
    friend class SkAnimateBase;
    friend class SkDisplayXMLParser;
    friend class SkAnimator;
    friend class SkAnimatorScript;
    friend class SkApply;
    friend class SkDisplayMovie;
    friend class SkDisplayType;
    friend class SkEvents;
    friend class SkGroup;
    friend struct SkMemberInfo;
};

#endif // SkAnimateMaker_DEFINED
