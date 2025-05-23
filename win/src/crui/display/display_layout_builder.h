// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_DISPLAY_DISPLAY_LAYOUT_BUILDER_H_
#define UI_DISPLAY_DISPLAY_LAYOUT_BUILDER_H_

#include <memory>

#include "crui/display/display_layout.h"
#include "crui/base/ui_export.h"

namespace crui {
namespace display {

class DisplayLayout;

// A utility class to create a DisplayLayout instance.
class CRUI_EXPORT DisplayLayoutBuilder final {
 public:
  DisplayLayoutBuilder(const DisplayLayoutBuilder&) = delete;
  DisplayLayoutBuilder& operator=(const DisplayLayoutBuilder&) = delete;

  // Creates a builder that uses a copy of the |layout| as a source.
  explicit DisplayLayoutBuilder(const DisplayLayout& layout);

  // Creates a builder with the primary display id.
  explicit DisplayLayoutBuilder(int64_t primary_id);

  ~DisplayLayoutBuilder();

  DisplayLayoutBuilder& SetDefaultUnified(bool default_unified);

  DisplayLayoutBuilder& SetMirrored(bool mirrored);

  DisplayLayoutBuilder& ClearPlacements();

  // Adds a display placement.
  DisplayLayoutBuilder& AddDisplayPlacement(int64_t display_id,
                                            int64_t parent_id,
                                            DisplayPlacement::Position position,
                                            int offset);

  // Adds a display placement.
  DisplayLayoutBuilder& AddDisplayPlacement(const DisplayPlacement& placement);

  // Sets the display placement for the secondary display.
  DisplayLayoutBuilder& SetSecondaryPlacement(
      int64_t secondary_id,
      DisplayPlacement::Position position,
      int offset);

  // Returns the DisplayLayout. After this call, the builder becomes invalid.
  std::unique_ptr<DisplayLayout> Build();

 private:
  std::unique_ptr<DisplayLayout> layout_;
};

}  // namespace display
}  // namespace crui

#endif  // UI_DISPLAY_DISPLAY_LAYOUT_BUILDER_H_
