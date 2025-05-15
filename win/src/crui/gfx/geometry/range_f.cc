// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/gfx/geometry/range_f.h"

#include <stddef.h>

#include <algorithm>
#include <cmath>

///#include "crbase/format_macros.h"
#include "crbase/strings/stringprintf.h"

namespace crui {
namespace gfx {

RangeF RangeF::Intersect(const RangeF& range) const {
  float min = std::max(GetMin(), range.GetMin());
  float max = std::min(GetMax(), range.GetMax());

  if (min >= max)  // No intersection.
    return InvalidRange();

  return RangeF(min, max);
}

RangeF RangeF::Intersect(const Range& range) const {
  RangeF range_f(static_cast<float>(range.start()), 
                 static_cast<float>(range.end()));
  return Intersect(range_f);
}

Range RangeF::Floor() const {
  uint32_t start = start_ > 0 ? static_cast<uint32_t>(std::floor(start_)) : 0;
  uint32_t end = end_ > 0 ? static_cast<uint32_t>(std::floor(end_)) : 0;
  return Range(start, end);
}

Range RangeF::Ceil() const {
  uint32_t start = start_ > 0 ? static_cast<uint32_t>(std::ceil(start_)) : 0;
  uint32_t end = end_ > 0 ? static_cast<uint32_t>(std::ceil(end_)) : 0;
  return Range(start, end);
}

Range RangeF::Round() const {
  uint32_t start = start_ > 0 ? static_cast<uint32_t>(std::round(start_)) : 0;
  uint32_t end = end_ > 0 ? static_cast<uint32_t>(std::round(end_)) : 0;
  return Range(start, end);
}

std::string RangeF::ToString() const {
  return cr::StringPrintf("{%f,%f}", start(), end());
}

}  // namespace gfx
}  // namespace crui
