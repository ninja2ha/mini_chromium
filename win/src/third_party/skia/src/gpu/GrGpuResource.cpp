
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrGpuResource.h"
#include "include/gpu/GrContext.h"
#include "src/gpu/GrResourceCache.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "include/core/SkTraceMemoryDump.h"

static inline GrResourceCache* get_resource_cache(GrGpu* gpu) {
    SkASSERT(gpu);
    SkASSERT(gpu->getContext());
    SkASSERT(gpu->getContext()->getResourceCache());
    return gpu->getContext()->getResourceCache();
}

GrGpuResource::GrGpuResource(GrGpu* gpu, LifeCycle lifeCycle)
    : fGpu(gpu)
    , fGpuMemorySize(kInvalidGpuMemorySize)
    , fLifeCycle(lifeCycle)
    , fUniqueID(CreateUniqueID()) {
    SkDEBUGCODE(fCacheArrayIndex = -1);
}

void GrGpuResource::registerWithCache() {
    get_resource_cache(fGpu)->resourceAccess().insertResource(this);
}

GrGpuResource::~GrGpuResource() {
    // The cache should have released or destroyed this resource.
    SkASSERT(this->wasDestroyed());
}

void GrGpuResource::release() { 
    SkASSERT(fGpu);
    this->onRelease();
    get_resource_cache(fGpu)->resourceAccess().removeResource(this);
    fGpu = nullptr;
    fGpuMemorySize = 0;
}

void GrGpuResource::abandon() {
    if (this->wasDestroyed()) {
        return;
    }
    SkASSERT(fGpu);
    this->onAbandon();
    get_resource_cache(fGpu)->resourceAccess().removeResource(this);
    fGpu = nullptr;
    fGpuMemorySize = 0;
}

void GrGpuResource::dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const {
    // Dump resource as "skia/gpu_resources/resource_#".
    SkString dumpName("skia/gpu_resources/resource_");
    dumpName.appendS32(this->getUniqueID());

    traceMemoryDump->dumpNumericValue(dumpName.c_str(), "size", "bytes", this->gpuMemorySize());

    if (this->isPurgeable()) {
        traceMemoryDump->dumpNumericValue(dumpName.c_str(), "purgeable_size", "bytes",
                                          this->gpuMemorySize());
    }

    // Call setMemoryBacking to allow sub-classes with implementation specific backings (such as GL
    // objects) to provide additional information.
    this->setMemoryBacking(traceMemoryDump, dumpName);
}

const SkData* GrGpuResource::setCustomData(const SkData* data) {
    SkSafeRef(data);
    fData.reset(data);
    return data;
}

const GrContext* GrGpuResource::getContext() const {
    if (fGpu) {
        return fGpu->getContext();
    } else {
        return nullptr;
    }
}

GrContext* GrGpuResource::getContext() {
    if (fGpu) {
        return fGpu->getContext();
    } else {
        return nullptr;
    }
}

void GrGpuResource::didChangeGpuMemorySize() const {
    if (this->wasDestroyed()) {
        return;
    }

    size_t oldSize = fGpuMemorySize;
    SkASSERT(kInvalidGpuMemorySize != oldSize);
    fGpuMemorySize = kInvalidGpuMemorySize;
    get_resource_cache(fGpu)->resourceAccess().didChangeGpuMemorySize(this, oldSize);
}

void GrGpuResource::removeUniqueKey() {
    if (this->wasDestroyed()) {
        return;
    }
    SkASSERT(fUniqueKey.isValid());
    get_resource_cache(fGpu)->resourceAccess().removeUniqueKey(this);
}

void GrGpuResource::setUniqueKey(const GrUniqueKey& key) {
    SkASSERT(this->internalHasRef());
    SkASSERT(key.isValid());

    // Wrapped and uncached resources can never have a unique key.
    if (!this->resourcePriv().isBudgeted()) {
        return;
    }

    if (this->wasDestroyed()) {
        return;
    }

    get_resource_cache(fGpu)->resourceAccess().changeUniqueKey(this, key);
}

void GrGpuResource::notifyAllCntsAreZero(CntType lastCntTypeToReachZero) const {
    if (this->wasDestroyed()) {
        // We've already been removed from the cache. Goodbye cruel world!
        delete this;
        return;
    }

    // We should have already handled this fully in notifyRefCntIsZero().
    SkASSERT(kRef_CntType != lastCntTypeToReachZero);

    GrGpuResource* mutableThis = const_cast<GrGpuResource*>(this);
    static const uint32_t kFlag =
        GrResourceCache::ResourceAccess::kAllCntsReachedZero_RefNotificationFlag;
    get_resource_cache(fGpu)->resourceAccess().notifyCntReachedZero(mutableThis, kFlag);
}

bool GrGpuResource::notifyRefCountIsZero() const {
    if (this->wasDestroyed()) {
        // handle this in notifyAllCntsAreZero().
        return true;
    }

    GrGpuResource* mutableThis = const_cast<GrGpuResource*>(this);
    uint32_t flags =
        GrResourceCache::ResourceAccess::kRefCntReachedZero_RefNotificationFlag;
    if (!this->internalHasPendingIO()) {
        flags |= GrResourceCache::ResourceAccess::kAllCntsReachedZero_RefNotificationFlag;
    }
    get_resource_cache(fGpu)->resourceAccess().notifyCntReachedZero(mutableThis, flags);

    // There is no need to call our notifyAllCntsAreZero function at this point since we already
    // told the cache about the state of cnts.
    return false;
}

void GrGpuResource::setScratchKey(const GrScratchKey& scratchKey) {
    SkASSERT(!fScratchKey.isValid());
    SkASSERT(scratchKey.isValid());
    // Wrapped resources can never have a scratch key.
    if (this->cacheAccess().isExternal()) {
        return;
    }
    fScratchKey = scratchKey;
}

void GrGpuResource::removeScratchKey() {
    if (!this->wasDestroyed() && fScratchKey.isValid()) {
        get_resource_cache(fGpu)->resourceAccess().willRemoveScratchKey(this);
        fScratchKey.reset();
    }
}

void GrGpuResource::makeBudgeted() {
    if (!this->wasDestroyed() && GrGpuResource::kUncached_LifeCycle == fLifeCycle) {
        fLifeCycle = kCached_LifeCycle;
        get_resource_cache(fGpu)->resourceAccess().didChangeBudgetStatus(this);
    }
}

void GrGpuResource::makeUnbudgeted() {
    if (!this->wasDestroyed() && GrGpuResource::kCached_LifeCycle == fLifeCycle &&
        !fUniqueKey.isValid()) {
        fLifeCycle = kUncached_LifeCycle;
        get_resource_cache(fGpu)->resourceAccess().didChangeBudgetStatus(this);
    }
}

uint32_t GrGpuResource::CreateUniqueID() {
    static int32_t gUniqueID = SK_InvalidUniqueID;
    uint32_t id;
    do {
        id = static_cast<uint32_t>(sk_atomic_inc(&gUniqueID) + 1);
    } while (id == SK_InvalidUniqueID);
    return id;
}
