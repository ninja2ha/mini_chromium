
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrResourceCache_DEFINED
#define GrResourceCache_DEFINED

#include "include/gpu/GrGpuResource.h"
#include "src/gpu/GrGpuResourceCacheAccess.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "include/gpu/GrResourceKey.h"
#include "src/core/SkMessageBus.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTArray.h"
#include "src/core/SkTDPQueue.h"
#include "include/core/SkTInternalLList.h"
#include "src/core/SkTMultiMap.h"

class GrCaps;
class SkString;
class SkTraceMemoryDump;

/**
 * Manages the lifetime of all GrGpuResource instances.
 *
 * Resources may have optionally have two types of keys:
 *      1) A scratch key. This is for resources whose allocations are cached but not their contents.
 *         Multiple resources can share the same scratch key. This is so a caller can have two
 *         resource instances with the same properties (e.g. multipass rendering that ping-pongs
 *         between two temporary surfaces). The scratch key is set at resource creation time and
 *         should never change. Resources need not have a scratch key.
 *      2) A unique key. This key's meaning is specific to the domain that created the key. Only one
 *         resource may have a given unique key. The unique key can be set, cleared, or changed
 *         anytime after resource creation.
 *
 * A unique key always takes precedence over a scratch key when a resource has both types of keys.
 * If a resource has neither key type then it will be deleted as soon as the last reference to it
 * is dropped.
 *
 * When proactive purging is enabled, on every flush, the timestamp of that flush is stored in a
 * n-sized ring buffer. When purging occurs each purgeable resource's timestamp is compared to the
 * timestamp of the n-th prior flush. If the resource's last use timestamp is older than the old
 * flush then the resource is proactively purged even when the cache is under budget. By default
 * this feature is disabled, though it can be enabled by calling GrResourceCache::setLimits.
 */
class GrResourceCache {
public:
    GrResourceCache(const GrCaps* caps);
    ~GrResourceCache();

    // Default maximum number of budgeted resources in the cache.
    static const int    kDefaultMaxCount            = 2 * (1 << 12);
    // Default maximum number of bytes of gpu memory of budgeted resources in the cache.
    static const size_t kDefaultMaxSize             = 96 * (1 << 20);
    // Default number of flushes a budgeted resources can go unused in the cache before it is
    // purged. Large values disable the feature (as the ring buffer of flush timestamps would be
    // large). This is currently the default until we decide to enable this feature
    // of the cache by default.
    static const int    kDefaultMaxUnusedFlushes    = 64;

    /** Used to access functionality needed by GrGpuResource for lifetime management. */
    class ResourceAccess;
    ResourceAccess resourceAccess();

    /**
     * Sets the cache limits in terms of number of resources, max gpu memory byte size, and number
     * of GrContext flushes that a resource can be unused before it is evicted. The latter value is
     * a suggestion and there is no promise that a resource will be purged immediately after it
     * hasn't been used in maxUnusedFlushes flushes.
     */
    void setLimits(int count, size_t bytes, int maxUnusedFlushes = kDefaultMaxUnusedFlushes);

    /**
     * Returns the number of resources.
     */
    int getResourceCount() const {
        return fPurgeableQueue.count() + fNonpurgeableResources.count();
    }

    /**
     * Returns the number of resources that count against the budget.
     */
    int getBudgetedResourceCount() const { return fBudgetedCount; }

    /**
     * Returns the number of bytes consumed by resources.
     */
    size_t getResourceBytes() const { return fBytes; }

    /**
     * Returns the number of bytes consumed by budgeted resources.
     */
    size_t getBudgetedResourceBytes() const { return fBudgetedBytes; }

    /**
     * Returns the cached resources count budget.
     */
    int getMaxResourceCount() const { return fMaxCount; }

    /**
     * Returns the number of bytes consumed by cached resources.
     */
    size_t getMaxResourceBytes() const { return fMaxBytes; }

    /**
     * Abandons the backend API resources owned by all GrGpuResource objects and removes them from
     * the cache.
     */
    void abandonAll();

    /**
     * Releases the backend API resources owned by all GrGpuResource objects and removes them from
     * the cache.
     */
    void releaseAll();

    enum {
        /** Preferentially returns scratch resources with no pending IO. */
        kPreferNoPendingIO_ScratchFlag = 0x1,
        /** Will not return any resources that match but have pending IO. */
        kRequireNoPendingIO_ScratchFlag = 0x2,
    };

    /**
     * Find a resource that matches a scratch key.
     */
    GrGpuResource* findAndRefScratchResource(const GrScratchKey& scratchKey,
                                             size_t resourceSize,
                                             uint32_t flags);
    
#ifdef SK_DEBUG
    // This is not particularly fast and only used for validation, so debug only.
    int countScratchEntriesForKey(const GrScratchKey& scratchKey) const {
        return fScratchMap.countForKey(scratchKey);
    }
#endif

    /**
     * Find a resource that matches a unique key.
     */
    GrGpuResource* findAndRefUniqueResource(const GrUniqueKey& key) {
        GrGpuResource* resource = fUniqueHash.find(key);
        if (resource) {
            this->refAndMakeResourceMRU(resource);
        }
        return resource;
    }

    /**
     * Query whether a unique key exists in the cache.
     */
    bool hasUniqueKey(const GrUniqueKey& key) const {
        return SkToBool(fUniqueHash.find(key));
    }

    /** Purges resources to become under budget and processes resources with invalidated unique
        keys. */
    void purgeAsNeeded();

    /** Purges all resources that don't have external owners. */
    void purgeAllUnlocked();

    /**
     * The callback function used by the cache when it is still over budget after a purge. The
     * passed in 'data' is the same 'data' handed to setOverbudgetCallback.
     */
    typedef void (*PFOverBudgetCB)(void* data);

    /**
     * Set the callback the cache should use when it is still over budget after a purge. The 'data'
     * provided here will be passed back to the callback. Note that the cache will attempt to purge
     * any resources newly freed by the callback.
     */
    void setOverBudgetCallback(PFOverBudgetCB overBudgetCB, void* data) {
        fOverBudgetCB = overBudgetCB;
        fOverBudgetData = data;
    }

    void notifyFlushOccurred();

#if GR_CACHE_STATS
    struct Stats {
        int fTotal;
        int fNumPurgeable;
        int fNumNonPurgeable;

        int fScratch;
        int fExternal;
        int fBorrowed;
        int fAdopted;
        size_t fUnbudgetedSize;

        Stats() { this->reset(); }

        void reset() {
            fTotal = 0;
            fNumPurgeable = 0;
            fNumNonPurgeable = 0;
            fScratch = 0;
            fExternal = 0;
            fBorrowed = 0;
            fAdopted = 0;
            fUnbudgetedSize = 0;
        }

        void update(GrGpuResource* resource) {
            if (resource->cacheAccess().isScratch()) {
                ++fScratch;
            }
            if (resource->cacheAccess().isExternal()) {
                ++fExternal;
            }
            if (resource->cacheAccess().isBorrowed()) {
                ++fBorrowed;
            }
            if (resource->cacheAccess().isAdopted()) {
                ++fAdopted;
            }
            if (!resource->resourcePriv().isBudgeted()) {
                fUnbudgetedSize += resource->gpuMemorySize();
            }
        }
    };

    void getStats(Stats*) const;

    void dumpStats(SkString*) const;

    void dumpStatsKeyValuePairs(SkTArray<SkString>* keys, SkTArray<double>* value) const;
#endif

    // This function is for unit testing and is only defined in test tools.
    void changeTimestamp(uint32_t newTimestamp);

    // Enumerates all cached resources and dumps their details to traceMemoryDump.
    void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const;

private:
    ///////////////////////////////////////////////////////////////////////////
    /// @name Methods accessible via ResourceAccess
    ////
    void insertResource(GrGpuResource*);
    void removeResource(GrGpuResource*);
    void notifyCntReachedZero(GrGpuResource*, uint32_t flags);
    void didChangeGpuMemorySize(const GrGpuResource*, size_t oldSize);
    void changeUniqueKey(GrGpuResource*, const GrUniqueKey&);
    void removeUniqueKey(GrGpuResource*);
    void willRemoveScratchKey(const GrGpuResource*);
    void didChangeBudgetStatus(GrGpuResource*);
    void refAndMakeResourceMRU(GrGpuResource*);
    /// @}

    void resetFlushTimestamps();
    void processInvalidUniqueKeys(const SkTArray<GrUniqueKeyInvalidatedMessage>&);
    void addToNonpurgeableArray(GrGpuResource*);
    void removeFromNonpurgeableArray(GrGpuResource*);
    bool overBudget() const { return fBudgetedBytes > fMaxBytes || fBudgetedCount > fMaxCount; }

    bool wouldFit(size_t bytes) {
        return fBudgetedBytes+bytes <= fMaxBytes && fBudgetedCount+1 <= fMaxCount;    
    }

    uint32_t getNextTimestamp();

#ifdef SK_DEBUG
    bool isInCache(const GrGpuResource* r) const;
    void validate() const;
#else
    void validate() const {}
#endif

    class AutoValidate;

    class AvailableForScratchUse;

    struct ScratchMapTraits {
        static const GrScratchKey& GetKey(const GrGpuResource& r) {
            return r.resourcePriv().getScratchKey();
        }

        static uint32_t Hash(const GrScratchKey& key) { return key.hash(); }
    };
    typedef SkTMultiMap<GrGpuResource, GrScratchKey, ScratchMapTraits> ScratchMap;

    struct UniqueHashTraits {
        static const GrUniqueKey& GetKey(const GrGpuResource& r) { return r.getUniqueKey(); }

        static uint32_t Hash(const GrUniqueKey& key) { return key.hash(); }
    };
    typedef SkTDynamicHash<GrGpuResource, GrUniqueKey, UniqueHashTraits> UniqueHash;

    static bool CompareTimestamp(GrGpuResource* const& a, GrGpuResource* const& b) {
        return a->cacheAccess().timestamp() < b->cacheAccess().timestamp();
    }

    static int* AccessResourceIndex(GrGpuResource* const& res) {
        return res->cacheAccess().accessCacheIndex();
    }

    typedef SkMessageBus<GrUniqueKeyInvalidatedMessage>::Inbox InvalidUniqueKeyInbox;
    typedef SkTDPQueue<GrGpuResource*, CompareTimestamp, AccessResourceIndex> PurgeableQueue;
    typedef SkTDArray<GrGpuResource*> ResourceArray;

    // Whenever a resource is added to the cache or the result of a cache lookup, fTimestamp is
    // assigned as the resource's timestamp and then incremented. fPurgeableQueue orders the
    // purgeable resources by this value, and thus is used to purge resources in LRU order.
    uint32_t                            fTimestamp;
    PurgeableQueue                      fPurgeableQueue;
    ResourceArray                       fNonpurgeableResources;

    // This map holds all resources that can be used as scratch resources.
    ScratchMap                          fScratchMap;
    // This holds all resources that have unique keys.
    UniqueHash                          fUniqueHash;

    // our budget, used in purgeAsNeeded()
    int                                 fMaxCount;
    size_t                              fMaxBytes;
    int                                 fMaxUnusedFlushes;

#if GR_CACHE_STATS
    int                                 fHighWaterCount;
    size_t                              fHighWaterBytes;
    int                                 fBudgetedHighWaterCount;
    size_t                              fBudgetedHighWaterBytes;
#endif

    // our current stats for all resources
    SkDEBUGCODE(int                     fCount;)
    size_t                              fBytes;

    // our current stats for resources that count against the budget
    int                                 fBudgetedCount;
    size_t                              fBudgetedBytes;

    PFOverBudgetCB                      fOverBudgetCB;
    void*                               fOverBudgetData;

    // We keep track of the "timestamps" of the last n flushes. If a resource hasn't been used in
    // that time then we well preemptively purge it to reduce memory usage.
    uint32_t*                           fFlushTimestamps;
    int                                 fLastFlushTimestampIndex;

    InvalidUniqueKeyInbox               fInvalidUniqueKeyInbox;

    // This resource is allowed to be in the nonpurgeable array for the sake of validate() because
    // we're in the midst of converting it to purgeable status.
    SkDEBUGCODE(GrGpuResource*          fNewlyPurgeableResourceForValidation;)

    bool                                fPreferVRAMUseOverFlushes;
};

class GrResourceCache::ResourceAccess {
private:
    ResourceAccess(GrResourceCache* cache) : fCache(cache) { }
    ResourceAccess(const ResourceAccess& that) : fCache(that.fCache) { }
    ResourceAccess& operator=(const ResourceAccess&); // unimpl

    /**
     * Insert a resource into the cache.
     */
    void insertResource(GrGpuResource* resource) { fCache->insertResource(resource); }

    /**
     * Removes a resource from the cache.
     */
    void removeResource(GrGpuResource* resource) { fCache->removeResource(resource); }

    /**
     * Notifications that should be sent to the cache when the ref/io cnt status of resources
     * changes.
     */
    enum RefNotificationFlags {
        /** All types of refs on the resource have reached zero. */
        kAllCntsReachedZero_RefNotificationFlag = 0x1,
        /** The normal (not pending IO type) ref cnt has reached zero. */
        kRefCntReachedZero_RefNotificationFlag  = 0x2,
    };
    /**
     * Called by GrGpuResources when they detect that their ref/io cnts have reached zero. When the
     * normal ref cnt reaches zero the flags that are set should be:
     *     a) kRefCntReachedZero if a pending IO cnt is still non-zero.
     *     b) (kRefCntReachedZero | kAllCntsReachedZero) when all pending IO cnts are also zero.
     * kAllCntsReachedZero is set by itself if a pending IO cnt is decremented to zero and all the
     * the other cnts are already zero.
     */
    void notifyCntReachedZero(GrGpuResource* resource, uint32_t flags) {
        fCache->notifyCntReachedZero(resource, flags);
    }

    /**
     * Called by GrGpuResources when their sizes change.
     */
    void didChangeGpuMemorySize(const GrGpuResource* resource, size_t oldSize) {
        fCache->didChangeGpuMemorySize(resource, oldSize);
    }

    /**
     * Called by GrGpuResources to change their unique keys.
     */
    void changeUniqueKey(GrGpuResource* resource, const GrUniqueKey& newKey) {
         fCache->changeUniqueKey(resource, newKey);
    }

    /**
     * Called by a GrGpuResource to remove its unique key.
     */
    void removeUniqueKey(GrGpuResource* resource) { fCache->removeUniqueKey(resource); }

    /**
     * Called by a GrGpuResource when it removes its scratch key.
     */
    void willRemoveScratchKey(const GrGpuResource* resource) {
        fCache->willRemoveScratchKey(resource);
    }

    /**
     * Called by GrGpuResources when they change from budgeted to unbudgeted or vice versa.
     */
    void didChangeBudgetStatus(GrGpuResource* resource) { fCache->didChangeBudgetStatus(resource); }

    // No taking addresses of this type.
    const ResourceAccess* operator&() const;
    ResourceAccess* operator&();

    GrResourceCache* fCache;

    friend class GrGpuResource; // To access all the proxy inline methods.
    friend class GrResourceCache; // To create this type.
};

inline GrResourceCache::ResourceAccess GrResourceCache::resourceAccess() {
    return ResourceAccess(this);
}

#endif
