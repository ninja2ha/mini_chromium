// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_CURSOR_CURSOR_LOADER_H_
#define UI_BASE_CURSOR_CURSOR_LOADER_H_

#include "crbase/logging.h"
#include "crbase/strings/string16.h"
#include "crui/base/cursor/cursor_types.h"
#include "crui/base/ui_export.h"
#include "crui/display/display.h"
#include "crui/gfx/geometry/point.h"
#include "crui/gfx/native_widget_types.h"

namespace crui {

class CRUI_EXPORT CursorLoader {
 public:
  CursorLoader(const CursorLoader&) = delete;
  CursorLoader& operator=(const CursorLoader&) = delete;

  CursorLoader() : scale_(1.f), rotation_(display::Display::ROTATE_0) {}
  virtual ~CursorLoader() {}

  display::Display::Rotation rotation() const { return rotation_; }

  void set_rotation(display::Display::Rotation rotation) {
    rotation_ = rotation;
  }

  // Returns the current scale of the mouse cursor icon.
  float scale() const {
    return scale_;
  }

  // Sets the scale of the mouse cursor icon.
  void set_scale(const float scale) {
    scale_ = scale;
  }

  // Creates a cursor from an image resource and puts it in the cursor map.
  virtual void LoadImageCursor(CursorType id,
                               int resource_id,
                               const gfx::Point& hot) = 0;

  // Creates an animated cursor from an image resource and puts it in the
  // cursor map. The image is assumed to be a concatenation of animation frames
  // from left to right. Also, each frame is assumed to be square
  // (width == height).
  // |frame_delay_ms| is the delay between frames in millisecond.
  virtual void LoadAnimatedCursor(CursorType id,
                                  int resource_id,
                                  const gfx::Point& hot,
                                  int frame_delay_ms) = 0;

  // Unloads all the cursors.
  virtual void UnloadAll() = 0;

  // Sets the platform cursor based on the native type of |cursor|.
  virtual void SetPlatformCursor(gfx::NativeCursor* cursor) = 0;

  // Creates a CursorLoader.
  static CursorLoader* Create();

 private:
  // The current scale of the mouse cursor icon.
  float scale_;

  // The current rotation of the mouse cursor icon.
  display::Display::Rotation rotation_;
};

}  // namespace crui

#endif  // UI_BASE_CURSOR_CURSOR_LOADER_H_
