// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/gfx/path_win.h"

#include <stdint.h>

#include "crui/base/win/scoped_gdi_object.h"
#include "third_party/skia/include/core/SkRegion.h"
#include "crui/gfx/path.h"

namespace gfx {

HRGN CreateHRGNFromSkRegion(const SkRegion& region) {
  crui::win::ScopedRegion temp(::CreateRectRgn(0, 0, 0, 0));
  crui::win::ScopedRegion result(::CreateRectRgn(0, 0, 0, 0));

  for (SkRegion::Iterator i(region); !i.done(); i.next()) {
    const SkIRect& rect = i.rect();
    ::SetRectRgn(temp.get(),
                 rect.left(), rect.top(), rect.right(), rect.bottom());
    ::CombineRgn(result.get(), result.get(), temp.get(), RGN_OR);
  }

  return result.release();
}

HRGN CreateHRGNFromSkPath(const SkPath& path) {
  SkRegion clip_region;
  clip_region.setRect(path.getBounds().round());
  SkRegion region;
  region.setPath(path, clip_region);
  return CreateHRGNFromSkRegion(region);
}

}  // namespace gfx
