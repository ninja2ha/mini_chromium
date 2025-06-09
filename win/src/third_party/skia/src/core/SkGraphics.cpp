
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "include/core/SkGraphics.h"

#include "src/core/SkBlitter.h"
#include "include/core/SkCanvas.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkGlyphCache.h"
#include "include/core/SkMath.h"
#include "include/core/SkMatrix.h"
#include "src/core/SkOpts.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPixelRef.h"
#include "include/core/SkRefCnt.h"
#include "src/core/SkResourceCache.h"
#include "include/utils/SkRTConf.h"
#include "src/core/SkScalerContext.h"
#include "include/core/SkShader.h"
#include "include/core/SkStream.h"
#include "include/private/SkTSearch.h"
#include "include/core/SkTime.h"
#include "include/core/SkUtils.h"
#include "include/core/SkXfermode.h"

#include <stdlib.h>

void SkGraphics::GetVersion(int32_t* major, int32_t* minor, int32_t* patch) {
    if (major) {
        *major = SKIA_VERSION_MAJOR;
    }
    if (minor) {
        *minor = SKIA_VERSION_MINOR;
    }
    if (patch) {
        *patch = SKIA_VERSION_PATCH;
    }
}

void SkGraphics::Init() {
    // SkGraphics::Init() must be thread-safe and idempotent.
    SkOpts::Init();

#ifdef SK_DEVELOPER
    skRTConfRegistry().possiblyDumpFile();
    skRTConfRegistry().validate();
    if (skRTConfRegistry().hasNonDefault()) {
        SkDebugf("Non-default runtime configuration options:\n");
        skRTConfRegistry().printNonDefault();
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////

void SkGraphics::DumpMemoryStatistics(SkTraceMemoryDump* dump) {
  SkResourceCache::DumpMemoryStatistics(dump);
  SkGlyphCache::DumpMemoryStatistics(dump);
}

///////////////////////////////////////////////////////////////////////////////

static const char kFontCacheLimitStr[] = "font-cache-limit";
static const size_t kFontCacheLimitLen = sizeof(kFontCacheLimitStr) - 1;

static const struct {
    const char* fStr;
    size_t fLen;
    size_t (*fFunc)(size_t);
} gFlags[] = {
    { kFontCacheLimitStr, kFontCacheLimitLen, SkGraphics::SetFontCacheLimit }
};

/* flags are of the form param; or param=value; */
void SkGraphics::SetFlags(const char* flags) {
    if (!flags) {
        return;
    }
    const char* nextSemi;
    do {
        size_t len = strlen(flags);
        const char* paramEnd = flags + len;
        const char* nextEqual = strchr(flags, '=');
        if (nextEqual && paramEnd > nextEqual) {
            paramEnd = nextEqual;
        }
        nextSemi = strchr(flags, ';');
        if (nextSemi && paramEnd > nextSemi) {
            paramEnd = nextSemi;
        }
        size_t paramLen = paramEnd - flags;
        for (int i = 0; i < (int)SK_ARRAY_COUNT(gFlags); ++i) {
            if (paramLen != gFlags[i].fLen) {
                continue;
            }
            if (strncmp(flags, gFlags[i].fStr, paramLen) == 0) {
                size_t val = 0;
                if (nextEqual) {
                    val = (size_t) atoi(nextEqual + 1);
                }
                (gFlags[i].fFunc)(val);
                break;
            }
        }
        flags = nextSemi + 1;
    } while (nextSemi);
}
