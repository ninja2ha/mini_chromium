// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_SCOPED_CANVAS_H_
#define UI_GFX_SCOPED_CANVAS_H_

#include "crui/gfx/canvas.h"

namespace crui {
namespace gfx {

// Saves the drawing state, and restores the state when going out of scope.
class ScopedCanvas {
 public:
  ScopedCanvas(const ScopedCanvas&) = delete;
  ScopedCanvas& operator=(const ScopedCanvas&) = delete;

  explicit ScopedCanvas(gfx::Canvas* canvas) : canvas_(canvas) {
    if (canvas_)
      canvas_->Save();
  }
  ~ScopedCanvas() {
    if (canvas_)
      canvas_->Restore();
  }

 private:
  gfx::Canvas* canvas_;
};

}  // namespace gfx
}  // namespace crui

#endif  // UI_GFX_SCOPED_CANVAS_H_
