// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_LAYOUT_LAYOUT_TYPES_H_
#define UI_VIEWS_LAYOUT_LAYOUT_TYPES_H_

#include <ostream>
#include <string>
#include <tuple>

#include "crbase/containers/optional.h"
#include "crui/gfx/geometry/size.h"
#include "crui/base/ui_export.h"

namespace crui {

namespace views {

// Whether a layout is oriented horizontally or vertically.
enum class LayoutOrientation {
  kHorizontal,
  kVertical,
};

// Stores an optional width and height upper bound. Used when calculating the
// preferred size of a layout pursuant to a maximum available size.
class CRUI_EXPORT SizeBounds {
 public:
  // Method definitions below to avoid "complex constructor" warning.  Marked
  // explicitly inline because Clang currently doesn't realize that "constexpr"
  // explicitly means "inline" and thus should count as "intentionally inlined
  // and thus shouldn't be warned about".
  // TODO(crbug.com/1045568): Remove "inline" if Clang's isInlineSpecified()
  // learns about constexpr.
  // TODO(crbug.com/1045570): Put method bodies here if complex constructor
  // heuristic learns to peer into types to discover that e.g. Optional is not
  // complex.
  SizeBounds() = default;
  SizeBounds(cr::Optional<int> width,
             cr::Optional<int> height);
  explicit SizeBounds(const gfx::Size& size);
  SizeBounds(const SizeBounds&) = default;
  SizeBounds(SizeBounds&&) = default;
  SizeBounds& operator=(const SizeBounds&) = default;
  SizeBounds& operator=(SizeBounds&&) = default;
  ~SizeBounds() = default;

  const cr::Optional<int>& width() const { return width_; }
  void set_width(cr::Optional<int> width) {
    width_ = std::move(width);
  }

  const cr::Optional<int>& height() const { return height_; }
  void set_height(cr::Optional<int> height) {
    height_ = std::move(height);
  }

  // Enlarges (or shrinks, if negative) each upper bound that is present by the
  // specified amounts.
  void Enlarge(int width, int height);

  std::string ToString() const;

 private:
  cr::Optional<int> width_;
  cr::Optional<int> height_;
};

CRUI_EXPORT bool operator==(const SizeBounds& lhs, const SizeBounds& rhs);
CRUI_EXPORT bool operator!=(const SizeBounds& lhs, const SizeBounds& rhs);
CRUI_EXPORT bool operator<(const SizeBounds& lhs, const SizeBounds& rhs);

}  // namespace views
}  // namespace crui

#endif  // UI_VIEWS_LAYOUT_LAYOUT_TYPES_H_
