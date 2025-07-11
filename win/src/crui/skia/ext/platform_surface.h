// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKIA_EXT_PLATFORM_SURFACE_H_
#define SKIA_EXT_PLATFORM_SURFACE_H_

#include "crbase/build_platform.h"

#include "third_party/skia/include/core/SkTypes.h"
#include "third_party/skia/include/core/SkRect.h"

#if defined(MINI_CHROMIUM_USE_CAIRO)
typedef struct _cairo cairo_t;
typedef struct _cairo_rectangle cairo_rectangle_t;
#elif defined(MINI_CHROMIUM_OS_MACOSX)
typedef struct CGContext* CGContextRef;
typedef struct CGRect CGRect;
#endif

namespace skia {

#if defined(MINI_CHROMIUM_OS_WIN)
typedef HDC PlatformSurface;
typedef RECT PlatformRect;
#elif defined(MINI_CHROMIUM_USE_CAIRO)
typedef cairo_t* PlatformSurface;
typedef cairo_rectangle_t PlatformRect;
#elif defined(MINI_CHROMIUM_OS_MACOSX)
typedef CGContextRef PlatformSurface;
typedef CGRect PlatformRect;
#else
typedef void* PlatformSurface;
typedef SkIRect* PlatformRect;
#endif

}  // namespace skia

#endif  // SKIA_EXT_PLATFORM_SURFACE_H_
