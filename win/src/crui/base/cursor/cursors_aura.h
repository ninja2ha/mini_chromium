// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_CURSOR_CURSORS_AURA_H_
#define UI_BASE_CURSOR_CURSORS_AURA_H_

#include "crui/base/cursor/cursor.h"
#include "crui/base/ui_export.h"

namespace crui {

namespace gfx {
class Point;
}

const int kAnimatedCursorFrameDelayMs = 25;

// Returns CSS cursor name from an Aura cursor ID.
CRUI_EXPORT const char* CursorCssNameFromId(CursorType id);

// Returns data about |id|, where id is a cursor constant like
// ui::CursorType::kHelp. The IDR will be placed in |resource_id| and the
// hotspots for the different DPIs will be placed in |hot_1x| and |hot_2x|.
// Returns false if |id| is invalid.
CRUI_EXPORT bool GetCursorDataFor(CursorSize cursor_size,
                                  CursorType id,
                                  float scale_factor,
                                  int* resource_id,
                                  gfx::Point* point);

// Like above, but for animated cursors.
CRUI_EXPORT bool GetAnimatedCursorDataFor(CursorSize cursor_size,
                                          CursorType id,
                                          float scale_factor,
                                          int* resource_id,
                                          gfx::Point* point);

}  // namespace crui

#endif  // UI_BASE_CURSOR_CURSORS_AURA_H_
