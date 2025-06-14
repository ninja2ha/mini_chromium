// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKIA_EXT_PLATFORM_DEVICE_H_
#define SKIA_EXT_PLATFORM_DEVICE_H_

#include "crbase/build_platform.h"

#if defined(MINI_CHROMIUM_OS_WIN)
#include <windows.h>
#include <vector>
#endif

#include "crui/skia/ext/platform_surface.h"
#include "third_party/skia/include/core/SkBitmapDevice.h"
#include "third_party/skia/include/core/SkTypes.h"

class SkMatrix;
class SkPath;
class SkRegion;

namespace skia {

class PlatformDevice;

// The following routines provide accessor points for the functionality
// exported by the various PlatformDevice ports.  
// All calls to PlatformDevice::* should be routed through these 
// helper functions.

// Bind a PlatformDevice instance, |platform_device| to |device|.  Subsequent
// calls to the functions exported below will forward the request to the
// corresponding method on the bound PlatformDevice instance.    If no
// PlatformDevice has been bound to the SkBaseDevice passed, then the 
// routines are NOPS.
SK_API void SetPlatformDevice(SkBaseDevice* device,
                              PlatformDevice* platform_device);
SK_API PlatformDevice* GetPlatformDevice(SkBaseDevice* device);

// A SkBitmapDevice is basically a wrapper around SkBitmap that provides a 
// surface for SkCanvas to draw into. PlatformDevice provides a surface 
// Windows can also write to. It also provides functionality to play well 
// with GDI drawing functions. This class is abstract and must be subclassed. 
// It provides the basic interface to implement it either with or without 
// a bitmap backend.
//
// PlatformDevice provides an interface which sub-classes of SkBaseDevice can 
// also provide to allow for drawing by the native platform into the device.
// TODO(robertphillips): Once the bitmap-specific entry points are removed
// from SkBaseDevice it might make sense for PlatformDevice to be derived
// from it.
class SK_API PlatformDevice {
 public:
  virtual ~PlatformDevice() {}

#if defined(MINI_CHROMIUM_OS_MACOSX)
  // The CGContext that corresponds to the bitmap, used for CoreGraphics
  // operations drawing into the bitmap. This is possibly heavyweight, so it
  // should exist only during one pass of rendering.
  virtual CGContextRef GetBitmapContext() = 0;
#endif

  // The DC that corresponds to the bitmap, used for GDI operations drawing
  // into the bitmap. This is possibly heavyweight, so it should be existant
  // only during one pass of rendering.
  virtual PlatformSurface BeginPlatformPaint();

  // Finish a previous call to beginPlatformPaint.
  virtual void EndPlatformPaint();

  // Returns true if GDI operations can be used for drawing into the bitmap.
  virtual bool SupportsPlatformPaint();

#if defined(MINI_CHROMIUM_OS_WIN)
  // Loads a SkPath into the GDI context. The path can there after be used for
  // clipping or as a stroke. Returns false if the path failed to be loaded.
  static bool LoadPathToDC(HDC context, const SkPath& path);

  // Loads a SkRegion into the GDI context.
  static void LoadClippingRegionToDC(HDC context, const SkRegion& region,
                                     const SkMatrix& transformation);

  // Draws to the given screen DC, if the bitmap DC doesn't exist, this will
  // temporarily create it. However, if you have created the bitmap DC, it will
  // be more efficient if you don't free it until after this call so it doesn't
  // have to be created twice.  If src_rect is null, then the entirety of the
  // source device will be copied.
  virtual void DrawToHDC(HDC, int x, int y, const RECT* src_rect);
#endif

 protected:
#if defined(MINI_CHROMIUM_OS_WIN)
  // Arrays must be inside structures.
  struct CubicPoints {
    SkPoint p[4];
  };
  typedef std::vector<CubicPoints> CubicPath;
  typedef std::vector<CubicPath> CubicPaths;

  // Loads the specified Skia transform into the device context, excluding
  // perspective (which GDI doesn't support).
  static void LoadTransformToDC(HDC dc, const SkMatrix& matrix);

  // Transforms SkPath's paths into a series of cubic path.
  static bool SkPathToCubicPaths(CubicPaths* paths, const SkPath& skpath);
#endif
};

}  // namespace skia

#endif  // SKIA_EXT_PLATFORM_DEVICE_H_
