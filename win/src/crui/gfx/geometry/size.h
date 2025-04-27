// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_GEOMETRY_SIZE_H_
#define UI_GFX_GEOMETRY_SIZE_H_

#include <algorithm>
#include <iosfwd>
#include <string>

#include "crbase/compiler_specific.h"
#include "crbase/numerics/safe_math.h"
#include "crbase/build_platform.h"

#include "crui/base/ui_export.h"

#if defined(MINI_CHROMIUM_OS_WIN)
typedef struct tagSIZE SIZE;
#elif defined(MINI_CHROMIUM_OS_MACOSX)
typedef struct CGSize CGSize;
#endif

namespace crui {
namespace gfx {

// A size has width and height values.
class CRUI_EXPORT Size {
 public:
  constexpr Size() : width_(0), height_(0) {}
  constexpr Size(int width, int height)
      : width_(std::max(0, width)), height_(std::max(0, height)) {}
#if defined(MINI_CHROMIUM_OS_MACOSX)
  explicit Size(const CGSize& s);
#endif

#if defined(MINI_CHROMIUM_OS_MACOSX)
  Size& operator=(const CGSize& s);
#endif

  void operator+=(const Size& size);

  void operator-=(const Size& size);

#if defined(MINI_CHROMIUM_OS_WIN)
  SIZE ToSIZE() const;
#elif defined(MINI_CHROMIUM_OS_MACOSX)
  CGSize ToCGSize() const;
#endif

  constexpr int width() const { return width_; }
  constexpr int height() const { return height_; }

  void set_width(int width) { width_ = std::max(0, width); }
  void set_height(int height) { height_ = std::max(0, height); }

  // This call will CHECK if the area of this size would overflow int.
  int GetArea() const;
  // Returns a checked numeric representation of the area.
  cr::CheckedNumeric<int> GetCheckedArea() const;

  void SetSize(int width, int height) {
    set_width(width);
    set_height(height);
  }

  void Enlarge(int grow_width, int grow_height);

  void SetToMin(const Size& other);
  void SetToMax(const Size& other);

  bool IsEmpty() const { return !width() || !height(); }

  std::string ToString() const;

 private:
  int width_;
  int height_;
};

inline bool operator==(const Size& lhs, const Size& rhs) {
  return lhs.width() == rhs.width() && lhs.height() == rhs.height();
}

inline bool operator!=(const Size& lhs, const Size& rhs) {
  return !(lhs == rhs);
}

inline Size operator+(Size lhs, const Size& rhs) {
  lhs += rhs;
  return lhs;
}

inline Size operator-(Size lhs, const Size& rhs) {
  lhs -= rhs;
  return lhs;
}

// Helper methods to scale a gfx::Size to a new gfx::Size.
CRUI_EXPORT Size ScaleToCeiledSize(const Size& size,
                                   float x_scale,
                                   float y_scale);
CRUI_EXPORT Size ScaleToCeiledSize(const Size& size, float scale);
CRUI_EXPORT Size ScaleToFlooredSize(const Size& size,
                                    float x_scale,
                                    float y_scale);
CRUI_EXPORT Size ScaleToFlooredSize(const Size& size, float scale);
CRUI_EXPORT Size ScaleToRoundedSize(const Size& size,
                                    float x_scale,
                                    float y_scale);
CRUI_EXPORT Size ScaleToRoundedSize(const Size& size, float scale);

}  // namespace gfx
}  // namespace crui

#endif  // UI_GFX_GEOMETRY_SIZE_H_
