// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKIA_EXT_BITMAP_PLATFORM_DEVICE_WIN_H_
#define SKIA_EXT_BITMAP_PLATFORM_DEVICE_WIN_H_

#include <windows.h>

#include "crbase/compiler_specific.h"
#include "crui/skia/ext/platform_device.h"
#include "crui/skia/ext/refptr.h"
#include "third_party/skia/include/core/SkBitmapDevice.h"

namespace skia {

// A device is basically a wrapper around SkBitmap that provides a surface for
// SkCanvas to draw into. Our device provides a surface Windows can also write
// to. BitmapPlatformDevice creates a bitmap using CreateDIBSection() in a
// format that Skia supports and can then use this to draw ClearType into, etc.
// This pixel data is provided to the bitmap that the device contains so that it
// can be shared.
class SK_API BitmapPlatformDevice : public SkBitmapDevice, public PlatformDevice {
 public:
  // Factory function. is_opaque should be set if the caller knows the bitmap
  // will be completely opaque and allows some optimizations.
  //
  // The |shared_section| parameter is optional (pass NULL for default
  // behavior). If |shared_section| is non-null, then it must be a handle to a
  // file-mapping object returned by CreateFileMapping.  See CreateDIBSection
  // for details. If |shared_section| is null, the bitmap backing store is not
  // initialized.
  static BitmapPlatformDevice* Create(int width, int height,
                                      bool is_opaque, HANDLE shared_section,
                                      bool do_clear = false);

  BitmapPlatformDevice(const BitmapPlatformDevice&) = delete;
  BitmapPlatformDevice& operator=(const BitmapPlatformDevice&) = delete;

  // Create a BitmapPlatformDevice with no shared section. The bitmap is not
  // initialized to 0.
  static BitmapPlatformDevice* Create(int width, int height, bool is_opaque);

  ~BitmapPlatformDevice() override;

  // PlatformDevice overrides
  // Retrieves the bitmap DC, which is the memory DC for our bitmap data. The
  // bitmap DC is lazy created.
  PlatformSurface BeginPlatformPaint() override;
  void EndPlatformPaint() override;

  // Loads the given transform and clipping region into the HDC. This is
  // overridden from SkBaseDevice.
  void setMatrixClip(const SkMatrix& transform,
                     const SkRegion& region,
                     const SkClipStack&) override;

  void DrawToHDC(HDC dc, int x, int y, const RECT* src_rect) override;

 protected:
  // Flushes the Windows device context so that the pixel data can be accessed
  // directly by Skia. Overridden from SkBaseDevice, this is called when Skia
  // starts accessing pixel data.
  const SkBitmap& onAccessBitmap() override;

  SkBaseDevice* onCreateDevice(const CreateInfo&, const SkPaint*) override;

 private:
  // Private constructor.
  BitmapPlatformDevice(HBITMAP hbitmap, const SkBitmap& bitmap);

  // Bitmap into which the drawing will be done. This bitmap not owned by this
  // class, but by the BitmapPlatformPixelRef inside the device's SkBitmap.
  // It's only stored here in order to lazy-create the DC (below).
  HBITMAP hbitmap_;

  // Previous bitmap held by the DC. This will be selected back before the
  // DC is destroyed.
  HBITMAP old_hbitmap_;

  // Lazily-created DC used to draw into the bitmap; see GetBitmapDC().
  HDC hdc_;

  // True when there is a transform or clip that has not been set to the
  // context.  The context is retrieved for every text operation, and the
  // transform and clip do not change as much. We can save time by not loading
  // the clip and transform for every one.
  bool config_dirty_;

  // Translation assigned to the context: we need to keep track of this
  // separately so it can be updated even if the context isn't created yet.
  SkMatrix transform_;

  // The current clipping region.
  SkRegion clip_region_;

  // Create/destroy hdc_, which is the memory DC for our bitmap data.
  HDC GetBitmapDC();
  void ReleaseBitmapDC();
  bool IsBitmapDCCreated() const;

  // Sets the transform and clip operations. This will not update the DC,
  // but will mark the config as dirty. The next call of LoadConfig will
  // pick up these changes.
  void SetMatrixClip(const SkMatrix& transform, const SkRegion& region);

  // Loads the current transform and clip into the context. Can be called even
  // when |hbitmap_| is NULL (will be a NOP).
  void LoadConfig();

#ifdef SK_DEBUG
  int begin_paint_count_;
#endif
};

}  // namespace skia

#endif  // SKIA_EXT_BITMAP_PLATFORM_DEVICE_WIN_H_
