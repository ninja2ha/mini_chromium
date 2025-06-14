/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkTLS.h"

// enable to help debug TLS storage
//#define SK_TRACE_TLS_LIFETIME


#ifdef SK_TRACE_TLS_LIFETIME
    #include "include/private/SkAtomics.h"
    static int32_t gTLSRecCount;
#endif

struct SkTLSRec {
    SkTLSRec*           fNext;
    void*               fData;
    SkTLS::CreateProc   fCreateProc;
    SkTLS::DeleteProc   fDeleteProc;

#ifdef SK_TRACE_TLS_LIFETIME
    SkTLSRec() {
        int n = sk_atomic_inc(&gTLSRecCount);
        SkDebugf(" SkTLSRec[%d]\n", n);
    }
#endif

    ~SkTLSRec() {
        if (fDeleteProc) {
            fDeleteProc(fData);
        }
        // else we leak fData, or it will be managed by the caller

#ifdef SK_TRACE_TLS_LIFETIME
        int n = sk_atomic_dec(&gTLSRecCount);
        SkDebugf("~SkTLSRec[%d]\n", n - 1);
#endif
    }
};

void SkTLS::Destructor(void* ptr) {
#ifdef SK_TRACE_TLS_LIFETIME
    SkDebugf("SkTLS::Destructor(%p)\n", ptr);
#endif

    SkTLSRec* rec = (SkTLSRec*)ptr;
    do {
        SkTLSRec* next = rec->fNext;
        delete rec;
        rec = next;
    } while (rec);
}

void* SkTLS::Get(CreateProc createProc, DeleteProc deleteProc) {
    if (nullptr == createProc) {
        return nullptr;
    }

    void* ptr = SkTLS::PlatformGetSpecific(true);

    if (ptr) {
        const SkTLSRec* rec = (const SkTLSRec*)ptr;
        do {
            if (rec->fCreateProc == createProc) {
                SkASSERT(rec->fDeleteProc == deleteProc);
                return rec->fData;
            }
        } while ((rec = rec->fNext) != nullptr);
        // not found, so create a new one
    }

    // add a new head of our change
    SkTLSRec* rec = new SkTLSRec;
    rec->fNext = (SkTLSRec*)ptr;

    SkTLS::PlatformSetSpecific(rec);

    rec->fData = createProc();
    rec->fCreateProc = createProc;
    rec->fDeleteProc = deleteProc;
    return rec->fData;
}

void* SkTLS::Find(CreateProc createProc) {
    if (nullptr == createProc) {
        return nullptr;
    }

    void* ptr = SkTLS::PlatformGetSpecific(false);

    if (ptr) {
        const SkTLSRec* rec = (const SkTLSRec*)ptr;
        do {
            if (rec->fCreateProc == createProc) {
                return rec->fData;
            }
        } while ((rec = rec->fNext) != nullptr);
    }
    return nullptr;
}

void SkTLS::Delete(CreateProc createProc) {
    if (nullptr == createProc) {
        return;
    }

    void* ptr = SkTLS::PlatformGetSpecific(false);

    SkTLSRec* curr = (SkTLSRec*)ptr;
    SkTLSRec* prev = nullptr;
    while (curr) {
        SkTLSRec* next = curr->fNext;
        if (curr->fCreateProc == createProc) {
            if (prev) {
                prev->fNext = next;
            } else {
                // we have a new head of our chain
                SkTLS::PlatformSetSpecific(next);
            }
            delete curr;
            break;
        }
        prev = curr;
        curr = next;
    }
}
