// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/views/window/window_resize_utils.h"

#include <algorithm>

#include "crui/gfx/geometry/rect.h"
#include "crui/gfx/geometry/size.h"

namespace crui {
namespace views {

// static
void WindowResizeUtils::SizeMinMaxToAspectRatio(float aspect_ratio,
                                                gfx::Size* min_window_size,
                                                gfx::Size* max_window_size) {
  CR_DCHECK(aspect_ratio > 0.0f);

  // Calculate the height using the min-width and aspect ratio.
  int min_height = static_cast<int>(min_window_size->width() / aspect_ratio);
  if (min_height < min_window_size->height()) {
    // The supplied width is too small to honor the min size, so use the height
    // to determine the minimum width.
    min_window_size->set_width(
        static_cast<int>(min_window_size->height() * aspect_ratio));
  } else {
    min_window_size->set_height(min_height);
  }

  // Calculate the height using the max-width and aspect ratio.
  int max_height = static_cast<int>(max_window_size->width() / aspect_ratio);
  if (max_height > max_window_size->height()) {
    // The supplied width is too large to honor the max size, so use the height
    // to determine the maximum width.
    max_window_size->set_width(
        static_cast<int>(max_window_size->height() * aspect_ratio));
  } else {
    max_window_size->set_height(max_height);
  }

  CR_DCHECK(max_window_size->width() >= min_window_size->width());
  CR_DCHECK(max_window_size->height() >= min_window_size->height());
}

// static
void WindowResizeUtils::SizeRectToAspectRatio(HitTest param,
                                              float aspect_ratio,
                                              const gfx::Size& min_window_size,
                                              const gfx::Size& max_window_size,
                                              gfx::Rect* rect) {
  CR_DCHECK(aspect_ratio > 0.0f);
  CR_DCHECK(max_window_size.width() >= min_window_size.width());
  CR_DCHECK(max_window_size.height() >= min_window_size.height());

  float rect_width = 0.0;
  float rect_height = 0.0;
  if (param == HitTest::kLeft || param == HitTest::kRight ||
      param == HitTest::kTopLeft ||
      param == HitTest::kBottomLeft) { /* horizontal axis to pivot */
    rect_width = static_cast<float>(
        std::min(max_window_size.width(),
                 std::max(rect->width(), min_window_size.width())));
    rect_height = rect_width / aspect_ratio;
  } else { /* vertical axis to pivot */
    rect_height = static_cast<float>(
        std::min(max_window_size.height(),
            std::max(rect->height(), min_window_size.height())));
    rect_width = rect_height * aspect_ratio;
  }

  // |rect| bounds before sizing to aspect ratio.
  int left = rect->x();
  int top = rect->y();
  int right = rect->right();
  int bottom = rect->bottom();

  switch (param) {
    case HitTest::kRight:
    case HitTest::kBottom:
      right = static_cast<int>(rect_width + left);
      bottom = static_cast<int>(top + rect_height);
      break;
    case HitTest::kTop:
      right = static_cast<int>(rect_width + left);
      top = static_cast<int>(bottom - rect_height);
      break;
    case HitTest::kLeft:
    case HitTest::kTopLeft:
      left = static_cast<int>(right - rect_width);
      top = static_cast<int>(bottom - rect_height);
      break;
    case HitTest::kTopRight:
      right = static_cast<int>(left + rect_width);
      top = static_cast<int>(bottom - rect_height);
      break;
    case HitTest::kBottomLeft:
      left = static_cast<int>(right - rect_width);
      bottom = static_cast<int>(top + rect_height);
      break;
    case HitTest::kBottomRight:
      right = static_cast<int>(left + rect_width);
      bottom = static_cast<int>(top + rect_height);
      break;
  }

  rect->SetByBounds(left, top, right, bottom);
}

}  // namespace views
}  // namespace crui
