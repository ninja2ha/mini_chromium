// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Defines a simple integer vector class.  This class is used to indicate a
// distance in two dimensions between two points. Subtracting two points should
// produce a vector, and adding a vector to a point produces the point at the
// vector's distance from the original point.

#ifndef UI_GFX_GEOMETRY_VECTOR2D_H_
#define UI_GFX_GEOMETRY_VECTOR2D_H_

#include <stdint.h>

#include <iosfwd>
#include <string>

#include "crui/base/ui_export.h"
#include "crui/gfx/geometry/vector2d_f.h"

namespace crui {
namespace gfx {

class CRUI_EXPORT Vector2d {
 public:
  constexpr Vector2d() : x_(0), y_(0) {}
  constexpr Vector2d(int x, int y) : x_(x), y_(y) {}

  constexpr int x() const { return x_; }
  constexpr float x_float() const { return static_cast<float>(x_); }
  void set_x(int x) { x_ = x; }

  constexpr int y() const { return y_; }
  constexpr float y_float() const { return static_cast<float>(y_); }
  void set_y(int y) { y_ = y; }

  // True if both components of the vector are 0.
  bool IsZero() const;

  // Add the components of the |other| vector to the current vector.
  void Add(const Vector2d& other);
  // Subtract the components of the |other| vector from the current vector.
  void Subtract(const Vector2d& other);

  constexpr bool operator==(const Vector2d& other) const {
    return x_ == other.x_ && y_ == other.y_;
  }
  void operator+=(const Vector2d& other) { Add(other); }
  void operator-=(const Vector2d& other) { Subtract(other); }

  void SetToMin(const Vector2d& other) {
    x_ = x_ <= other.x_ ? x_ : other.x_;
    y_ = y_ <= other.y_ ? y_ : other.y_;
  }

  void SetToMax(const Vector2d& other) {
    x_ = x_ >= other.x_ ? x_ : other.x_;
    y_ = y_ >= other.y_ ? y_ : other.y_;
  }

  // Gives the square of the diagonal length of the vector. Since this is
  // cheaper to compute than Length(), it is useful when you want to compare
  // relative lengths of different vectors without needing the actual lengths.
  int64_t LengthSquared() const;
  // Gives the diagonal length of the vector.
  float Length() const;

  std::string ToString() const;

  operator Vector2dF() const {
    return Vector2dF(static_cast<float>(x()), static_cast<float>(y()));
  }

 private:
  int x_;
  int y_;
};

inline constexpr Vector2d operator-(const Vector2d& v) {
  return Vector2d(-v.x(), -v.y());
}

inline Vector2d operator+(const Vector2d& lhs, const Vector2d& rhs) {
  Vector2d result = lhs;
  result.Add(rhs);
  return result;
}

inline Vector2d operator-(const Vector2d& lhs, const Vector2d& rhs) {
  Vector2d result = lhs;
  result.Add(-rhs);
  return result;
}

}  // namespace gfx
}  // namespace crui

#endif // UI_GFX_GEOMETRY_VECTOR2D_H_
