/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGraphics_DEFINED
#define SkGraphics_DEFINED

#include "include/core/SkTypes.h"

class SkData;
class SkImageGenerator;
class SkTraceMemoryDump;

class SK_API SkGraphics {
public:
    /**
     *  Call this at process initialization time if your environment does not
     *  permit static global initializers that execute code.
     *  Init() is thread-safe and idempotent.
     */
    static void Init();

    // We're in the middle of cleaning this up.
    static void Term() {}

    /**
     *  Return the version numbers for the library. If the parameter is not
     *  null, it is set to the version number.
     */
    static void GetVersion(int32_t* major, int32_t* minor, int32_t* patch);

    /**
     *  Return the max number of bytes that should be used by the font cache.
     *  If the cache needs to allocate more, it will purge previous entries.
     *  This max can be changed by calling SetFontCacheLimit().
     */
    static size_t GetFontCacheLimit();

    /**
     *  Specify the max number of bytes that should be used by the font cache.
     *  If the cache needs to allocate more, it will purge previous entries.
     *
     *  This function returns the previous setting, as if GetFontCacheLimit()
     *  had be called before the new limit was set.
     */
    static size_t SetFontCacheLimit(size_t bytes);

    /**
     *  Return the number of bytes currently used by the font cache.
     */
    static size_t GetFontCacheUsed();

    /**
     *  Return the number of entries in the font cache.
     *  A cache "entry" is associated with each typeface + pointSize + matrix.
     */
    static int GetFontCacheCountUsed();

    /**
     *  Return the current limit to the number of entries in the font cache.
     *  A cache "entry" is associated with each typeface + pointSize + matrix.
     */
    static int GetFontCacheCountLimit();

    /**
     *  Set the limit to the number of entries in the font cache, and return
     *  the previous value. If this new value is lower than the previous,
     *  it will automatically try to purge entries to meet the new limit.
     */
    static int SetFontCacheCountLimit(int count);

    /**
     *  For debugging purposes, this will attempt to purge the font cache. It
     *  does not change the limit, but will cause subsequent font measures and
     *  draws to be recreated, since they will no longer be in the cache.
     */
    static void PurgeFontCache();

    /**
     *  Scaling bitmaps with the kHigh_SkFilterQuality setting is
     *  expensive, so the result is saved in the global Scaled Image
     *  Cache.
     *
     *  This function returns the memory usage of the Scaled Image Cache.
     */
    static size_t GetResourceCacheTotalBytesUsed();

    /**
     *  These functions get/set the memory usage limit for the resource cache, used for temporary
     *  bitmaps and other resources. Entries are purged from the cache when the memory useage
     *  exceeds this limit.
     */
    static size_t GetResourceCacheTotalByteLimit();
    static size_t SetResourceCacheTotalByteLimit(size_t newLimit);

    /**
     *  For debugging purposes, this will attempt to purge the resource cache. It
     *  does not change the limit.
     */
    static void PurgeResourceCache();

    /**
     *  When the cachable entry is very lage (e.g. a large scaled bitmap), adding it to the cache
     *  can cause most/all of the existing entries to be purged. To avoid the, the client can set
     *  a limit for a single allocation. If a cacheable entry would have been cached, but its size
     *  exceeds this limit, then we do not attempt to cache it at all.
     *
     *  Zero is the default value, meaning we always attempt to cache entries.
     */
    static size_t GetResourceCacheSingleAllocationByteLimit();
    static size_t SetResourceCacheSingleAllocationByteLimit(size_t newLimit);

    /**
     *  Dumps memory usage of caches using the SkTraceMemoryDump interface. See SkTraceMemoryDump
     *  for usage of this method.
     */
    static void DumpMemoryStatistics(SkTraceMemoryDump* dump);

    /**
     *  Applications with command line options may pass optional state, such
     *  as cache sizes, here, for instance:
     *  font-cache-limit=12345678
     *
     *  The flags format is name=value[;name=value...] with no spaces.
     *  This format is subject to change.
     */
    static void SetFlags(const char* flags);

    /**
     *  Return the max number of bytes that should be used by the thread-local
     *  font cache.
     *  If the cache needs to allocate more, it will purge previous entries.
     *  This max can be changed by calling SetFontCacheLimit().
     *
     *  If this thread has never called SetTLSFontCacheLimit, or has called it
     *  with 0, then this thread is using the shared font cache. In that case,
     *  this function will always return 0, and the caller may want to call
     *  GetFontCacheLimit.
     */
    static size_t GetTLSFontCacheLimit();

    /**
     *  Specify the max number of bytes that should be used by the thread-local
     *  font cache. If this value is 0, then this thread will use the shared
     *  global font cache.
     */
    static void SetTLSFontCacheLimit(size_t bytes);

    typedef SkImageGenerator* (*ImageGeneratorFromEncodedFactory)(SkData*);

    /**
     *  To instantiate images from encoded data, first looks at this runtime function-ptr. If it
     *  exists, it is called to create an SkImageGenerator from SkData. If there is no function-ptr
     *  or there is, but it returns NULL, then skia will call its internal default implementation.
     *
     *  Returns the previous factory (which could be NULL).
     */
    static ImageGeneratorFromEncodedFactory
           SetImageGeneratorFromEncodedFactory(ImageGeneratorFromEncodedFactory);
};

class SkAutoGraphics {
public:
    SkAutoGraphics() {
        SkGraphics::Init();
    }
};

#endif
