
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "include/views/SkEventSink.h"
#include "include/private/SkMutex.h"
#include "src/views/SkTagList.h"
#include "include/core/SkTime.h"

class SkEventSink_Globals {
public:
    SkEventSink_Globals() {
        fNextSinkID = 0;
        fSinkHead = nullptr;
    }

    SkMutex         fSinkMutex;
    SkEventSinkID   fNextSinkID;
    SkEventSink*    fSinkHead;
};

static SkEventSink_Globals& getGlobals() {
    // leak this, so we don't incur any shutdown perf hit
    static SkEventSink_Globals* gGlobals = new SkEventSink_Globals;
    return *gGlobals;
}

SkEventSink::SkEventSink() : fTagHead(nullptr) {
    SkEventSink_Globals& globals = getGlobals();

    globals.fSinkMutex.acquire();

    fID = ++globals.fNextSinkID;
    fNextSink = globals.fSinkHead;
    globals.fSinkHead = this;

    globals.fSinkMutex.release();
}

SkEventSink::~SkEventSink() {
    SkEventSink_Globals& globals = getGlobals();

    if (fTagHead)
        SkTagList::DeleteAll(fTagHead);

    globals.fSinkMutex.acquire();

    SkEventSink* sink = globals.fSinkHead;
    SkEventSink* prev = nullptr;

    for (;;) {
        SkEventSink* next = sink->fNextSink;
        if (sink == this) {
            if (prev) {
                prev->fNextSink = next;
            } else {
                globals.fSinkHead = next;
            }
            break;
        }
        prev = sink;
        sink = next;
    }
    globals.fSinkMutex.release();
}

bool SkEventSink::doEvent(const SkEvent& evt) {
    return this->onEvent(evt);
}

bool SkEventSink::doQuery(SkEvent* evt) {
    SkASSERT(evt);
    return this->onQuery(evt);
}

bool SkEventSink::onEvent(const SkEvent&) {
    return false;
}

bool SkEventSink::onQuery(SkEvent*) {
    return false;
}

///////////////////////////////////////////////////////////////////////////////

SkTagList* SkEventSink::findTagList(U8CPU tag) const {
    return fTagHead ? SkTagList::Find(fTagHead, tag) : nullptr;
}

void SkEventSink::addTagList(SkTagList* rec) {
    SkASSERT(rec);
    SkASSERT(fTagHead == nullptr || SkTagList::Find(fTagHead, rec->fTag) == nullptr);

    rec->fNext = fTagHead;
    fTagHead = rec;
}

void SkEventSink::removeTagList(U8CPU tag) {
    if (fTagHead) {
        SkTagList::DeleteTag(&fTagHead, tag);
    }
}

///////////////////////////////////////////////////////////////////////////////

struct SkListenersTagList : SkTagList {
    SkListenersTagList(U16CPU count) : SkTagList(kListeners_SkTagList)
    {
        fExtra16 = SkToU16(count);
        fIDs = (SkEventSinkID*)sk_malloc_throw(count * sizeof(SkEventSinkID));
    }
    virtual ~SkListenersTagList()
    {
        sk_free(fIDs);
    }

    int countListners() const { return fExtra16; }

    int find(SkEventSinkID id) const
    {
        const SkEventSinkID* idptr = fIDs;
        for (int i = fExtra16 - 1; i >= 0; --i)
            if (idptr[i] == id)
                return i;
        return -1;
    }

    SkEventSinkID*  fIDs;
};

void SkEventSink::addListenerID(SkEventSinkID id)
{
    if (id == 0)
        return;

    SkListenersTagList* prev = (SkListenersTagList*)this->findTagList(kListeners_SkTagList);
    int                 count = 0;

    if (prev)
    {
        if (prev->find(id) >= 0)
            return;
        count = prev->countListners();
    }

    SkListenersTagList* next = new SkListenersTagList(count + 1);

    if (prev)
    {
        memcpy(next->fIDs, prev->fIDs, count * sizeof(SkEventSinkID));
        this->removeTagList(kListeners_SkTagList);
    }
    next->fIDs[count] = id;
    this->addTagList(next);
}

void SkEventSink::copyListeners(const SkEventSink& sink)
{
    SkListenersTagList* sinkList = (SkListenersTagList*)sink.findTagList(kListeners_SkTagList);
    if (sinkList == nullptr)
        return;
    SkASSERT(sinkList->countListners() > 0);
    const SkEventSinkID* iter = sinkList->fIDs;
    const SkEventSinkID* stop = iter + sinkList->countListners();
    while (iter < stop)
        addListenerID(*iter++);
}

void SkEventSink::removeListenerID(SkEventSinkID id)
{
    if (id == 0)
        return;

    SkListenersTagList* list = (SkListenersTagList*)this->findTagList(kListeners_SkTagList);

    if (list == nullptr)
        return;

    int index = list->find(id);
    if (index >= 0)
    {
        int count = list->countListners();
        SkASSERT(count > 0);
        if (count == 1)
            this->removeTagList(kListeners_SkTagList);
        else
        {
            // overwrite without resize/reallocating our struct (for speed)
            list->fIDs[index] = list->fIDs[count - 1];
            list->fExtra16 = SkToU16(count - 1);
        }
    }
}

bool SkEventSink::hasListeners() const
{
    return this->findTagList(kListeners_SkTagList) != nullptr;
}

void SkEventSink::postToListeners(const SkEvent& evt, SkMSec delay) {
    SkListenersTagList* list = (SkListenersTagList*)this->findTagList(kListeners_SkTagList);
    if (list) {
        SkASSERT(list->countListners() > 0);
        const SkEventSinkID* iter = list->fIDs;
        const SkEventSinkID* stop = iter + list->countListners();
        while (iter < stop) {
            SkEvent* copy = new SkEvent(evt);
            copy->setTargetID(*iter++)->postDelay(delay);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

SkEventSink::EventResult SkEventSink::DoEvent(const SkEvent& evt) {
    SkEvent::Proc proc = evt.getTargetProc();
    if (proc) {
        return proc(evt) ? kHandled_EventResult : kNotHandled_EventResult;
    }

    SkEventSink* sink = SkEventSink::FindSink(evt.getTargetID());
    if (sink) {
        return sink->doEvent(evt) ? kHandled_EventResult : kNotHandled_EventResult;
    }

    return kSinkNotFound_EventResult;
}

SkEventSink* SkEventSink::FindSink(SkEventSinkID sinkID)
{
    if (sinkID == 0)
        return 0;

    SkEventSink_Globals&    globals = getGlobals();
    SkAutoMutexAcquire      ac(globals.fSinkMutex);
    SkEventSink*            sink = globals.fSinkHead;

    while (sink)
    {
        if (sink->getSinkID() == sinkID)
            return sink;
        sink = sink->fNextSink;
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

#if 0   // experimental, not tested

#include "include/private/SkMutex.h"
#include "include/private/SkTDict.h"

#define kMinStringBufferSize    128
SK_DECLARE_STATIC_MUTEX(gNamedSinkMutex);
static SkTDict<SkEventSinkID>   gNamedSinkIDs(kMinStringBufferSize);

/** Register a name/id pair with the system. If the name already exists,
    replace its ID with the new id. This pair will persist until UnregisterNamedSink()
    is called.
*/
void SkEventSink::RegisterNamedSinkID(const char name[], SkEventSinkID id)
{
    if (id && name && *name)
    {
        SkAutoMutexAcquire  ac(gNamedSinkMutex);
        gNamedSinkIDs.set(name, id);
    }
}

/** Return the id that matches the specified name (from a previous call to
    RegisterNamedSinkID(). If no match is found, return 0
*/
SkEventSinkID SkEventSink::FindNamedSinkID(const char name[])
{
    SkEventSinkID id = 0;

    if (name && *name)
    {
        SkAutoMutexAcquire  ac(gNamedSinkMutex);
        (void)gNamedSinkIDs.find(name, &id);
    }
    return id;
}

/** Remove all name/id pairs from the system. This is call internally
    on shutdown, to ensure no memory leaks. It should not be called
    before shutdown.
*/
void SkEventSink::RemoveAllNamedSinkIDs()
{
    SkAutoMutexAcquire  ac(gNamedSinkMutex);
    (void)gNamedSinkIDs.reset();
}
#endif
