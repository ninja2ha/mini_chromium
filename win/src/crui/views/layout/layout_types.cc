// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/layout/layout_types.h"

#include "crbase/strings/strcat.h"
#include "crbase/strings/string_number_conversions.h"

namespace crui {
namespace views {

// SizeBounds ------------------------------------------------------------------

SizeBounds::SizeBounds(cr::Optional<int> width,
  cr::Optional<int> height)
  : width_(std::move(width)), height_(std::move(height)) {}

SizeBounds::SizeBounds(const gfx::Size& size)
  : width_(size.width()), height_(size.height()) {}

void SizeBounds::Enlarge(int width, int height) {
  if (width_)
    width_ = std::max(0, *width_ + width);
  if (height_)
    height_ = std::max(0, *height_ + height);
}

std::string SizeBounds::ToString() const {
  return cr::StrCat({width_ ? cr::NumberToString(*width_) : "_", " x ",
                     height_ ? cr::NumberToString(*height_) : "_"});
}

bool operator==(const SizeBounds& lhs, const SizeBounds& rhs) {
  return std::tie(lhs.width(), lhs.height()) ==
    std::tie(rhs.width(), rhs.height());
}

bool operator!=(const SizeBounds& lhs, const SizeBounds& rhs) {
  return !(lhs == rhs);
}

bool operator<(const SizeBounds& lhs, const SizeBounds& rhs) {
  return std::tie(lhs.height(), lhs.width()) <
    std::tie(rhs.height(), rhs.width());
}

}  // namespace views
}  // namespace crui
