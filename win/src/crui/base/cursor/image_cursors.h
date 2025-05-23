// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_CURSOR_IMAGE_CURSORS_H_
#define UI_BASE_CURSOR_IMAGE_CURSORS_H_

#include <memory>

#include "crbase/memory/weak_ptr.h"
#include "crbase/strings/string16.h"
#include "crui/base/cursor/cursor.h"
#include "crui/display/display.h"
#include "crui/gfx/native_widget_types.h"
#include "crui/base/ui_export.h"

namespace crui {

class CursorLoader;

// A utility class that provides cursors for NativeCursors for which we have
// image resources.
class CRUI_EXPORT ImageCursors {
 public:
  ImageCursors(const ImageCursors&) = delete;
  ImageCursors& operator=(const ImageCursors&) = delete;

  ImageCursors();
  ~ImageCursors();

  // Creates the |cursor_loader_|. This is optional as |cursor_loader_| is
  // lazily created if Initialize() isn't explictly called.
  // However note that it matters which thread is used to create
  // |cursor_loader_| (see CursorLoaderOzone,  crbug.com/741106). Thus explicit
  // call to Initialize may be useful to ensure initialization happens on the
  // right thread.
  void Initialize();

  // Returns the scale and rotation of the currently loaded cursor.
  float GetScale() const;
  display::Display::Rotation GetRotation() const;

  // Sets the display the cursors are loaded for. |scale_factor| determines the
  // size of the image to load. Returns true if the cursor image is reloaded.
  bool SetDisplay(const display::Display& display, float scale_factor);

  // Sets the size of the mouse cursor icon.
  void SetCursorSize(CursorSize cursor_size);

  // Sets the platform cursor based on the native type of |cursor|.
  void SetPlatformCursor(gfx::NativeCursor* cursor);

  cr::WeakPtr<ImageCursors> GetWeakPtr();

 private:
  // Reloads the all loaded cursors in the cursor loader.
  void ReloadCursors();

  std::unique_ptr<CursorLoader> cursor_loader_;
  CursorSize cursor_size_;
  cr::WeakPtrFactory<ImageCursors> weak_ptr_factory_{this};
};

}  // namespace crui

#endif  // UI_BASE_CURSOR_IMAGE_CURSORS_H_
