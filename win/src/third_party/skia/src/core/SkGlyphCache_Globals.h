/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGlyphCache_Globals_DEFINED
#define SkGlyphCache_Globals_DEFINED

#include "src/core/SkGlyphCache.h"
#include "include/private/SkMutex.h"
#include "include/private/SkSpinlock.h"
#include "src/core/SkTLS.h"

#ifndef SK_DEFAULT_FONT_CACHE_COUNT_LIMIT
    #define SK_DEFAULT_FONT_CACHE_COUNT_LIMIT   2048
#endif

#ifndef SK_DEFAULT_FONT_CACHE_LIMIT
    #define SK_DEFAULT_FONT_CACHE_LIMIT     (2 * 1024 * 1024)
#endif

///////////////////////////////////////////////////////////////////////////////

class SkGlyphCache_Globals {
public:
    SkGlyphCache_Globals() {
        fHead = nullptr;
        fTotalMemoryUsed = 0;
        fCacheSizeLimit = SK_DEFAULT_FONT_CACHE_LIMIT;
        fCacheCount = 0;
        fCacheCountLimit = SK_DEFAULT_FONT_CACHE_COUNT_LIMIT;
    }

    ~SkGlyphCache_Globals() {
        SkGlyphCache* cache = fHead;
        while (cache) {
            SkGlyphCache* next = cache->fNext;
            delete cache;
            cache = next;
        }
    }

    SkSpinlock     fLock;

    SkGlyphCache* internalGetHead() const { return fHead; }
    SkGlyphCache* internalGetTail() const;

    size_t getTotalMemoryUsed() const { return fTotalMemoryUsed; }
    int getCacheCountUsed() const { return fCacheCount; }

#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif

    int getCacheCountLimit() const { return fCacheCountLimit; }
    int setCacheCountLimit(int limit);

    size_t  getCacheSizeLimit() const { return fCacheSizeLimit; }
    size_t  setCacheSizeLimit(size_t limit);

    // returns true if this cache is over-budget either due to size limit
    // or count limit.
    bool isOverBudget() const {
        return fCacheCount > fCacheCountLimit ||
               fTotalMemoryUsed > fCacheSizeLimit;
    }

    void purgeAll(); // does not change budget

    // call when a glyphcache is available for caching (i.e. not in use)
    void attachCacheToHead(SkGlyphCache*);

    // can only be called when the mutex is already held
    void internalDetachCache(SkGlyphCache*);
    void internalAttachCacheToHead(SkGlyphCache*);

private:
    SkGlyphCache* fHead;
    size_t  fTotalMemoryUsed;
    size_t  fCacheSizeLimit;
    int32_t fCacheCountLimit;
    int32_t fCacheCount;

    // Checkout budgets, modulated by the specified min-bytes-needed-to-purge,
    // and attempt to purge caches to match.
    // Returns number of bytes freed.
    size_t internalPurge(size_t minBytesNeeded = 0);
};

#endif
