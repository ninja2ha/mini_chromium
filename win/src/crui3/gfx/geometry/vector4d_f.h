// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Defines a simple float vector class.  This class is used to indicate a
// distance in two dimensions between two points. Subtracting two points should
// produce a vector, and adding a vector to a point produces the point at the
// vector's distance from the original point.

#ifndef UI_GFX_GEOMETRY_VECTOR4D_F_H_
#define UI_GFX_GEOMETRY_VECTOR4D_F_H_

#include <iosfwd>
#include <string>

#include "crbase/compiler_specific.h"
#include "crbase/containers/span.h"
#include "crui/base/ui_export.h"
#include "crui/gfx/geometry/vector3d_f.h"

namespace crui {
namespace gfx {

class CRUI_EXPORT Vector4dF {
 public:
  constexpr Vector4dF() : vec_{0, 0, 0, 1} {}
  constexpr Vector4dF(double x, double y, double z, double w)
    : vec_{x, y, z, w} {}

  constexpr Vector4dF(const double v[4])
    : vec_{ v[0], v[1], v[2], v[3] } {}

  constexpr double x() const { return vec_[0]; }
  void set_x(double x) { vec_[0] = x; }

  constexpr double y() const { return vec_[1]; }
  void set_y(double y) { vec_[1] = y; }

  constexpr double z() const { return vec_[2]; }
  void set_z(double z) { vec_[2] = z; }

  constexpr double w() const { return vec_[3]; }
  void set_w(double w) { vec_[3] = w; }

  void operator=(const Vector4dF& v) {
    memcpy(vec_, v.vec_, sizeof(v.vec_));
  }

  inline cr::Span<double, 4> span(){
    return vec_;
  }

  // True if all components of the vector are 0.
  bool IsZero() const;

  // Add the components of the |other| vector to the current vector.
  void Add(const Vector4dF& other);
  // Subtract the components of the |other| vector from the current vector.
  void Subtract(const Vector4dF& other);

  void operator+=(const Vector4dF& other) { Add(other); }
  void operator-=(const Vector4dF& other) { Subtract(other); }
  void operator*=(const Vector4dF& other) { Scale(other); }
  void operator*=(double scale) {Scale(scale);}

  // Gives the square of the diagonal length of the vector.
  double LengthSquared() const;
  // Gives the diagonal length of the vector.
  double Length() const;

  // Scale all components of the vector by |scale|.
  void Scale(double scale) { Scale(scale, scale, scale, scale); }
  void Scale(const Vector4dF& other) { 
    Scale(other.x(), other.y(), other.z(), other.w()); 
  }
  // Scale the each component of the vector by the given scale factors.
  void Scale(double x_scale, double y_scale, double z_scale, double w_scale);

  void InvScale(double scale) { InvScale(scale, scale, scale, scale); }
  void InvScale(const Vector4dF& other) {
    InvScale(other.x(), other.y(), other.z(), other.w());
  }
  // Scale the each component of the vector by the given scale factors.
  void InvScale(double x_scale, double y_scale, double z_scale, double w_scale);

  // |out| is assigned a unit-length vector in the direction of |this| iff
  // this function returns true. It can return false if |this| is too short.
  bool GetNormalized(Vector4dF* out) const;

  std::string ToString() const;

 private:
  double vec_[4];
};

inline bool operator==(const Vector4dF& lhs, const Vector4dF& rhs) {
  return lhs.x() == rhs.x() && lhs.y() == rhs.y() && 
         lhs.z() == rhs.z() && lhs.w() == rhs.w();
}

inline bool operator!=(const Vector4dF& lhs, const Vector4dF& rhs) {
  return !(lhs == rhs);
}

inline Vector4dF operator-(const Vector4dF& v) {
  return Vector4dF(-v.x(), -v.y(), -v.z(), -v.w());
}

inline Vector4dF operator+(const Vector4dF& lhs, const Vector4dF& rhs) {
  Vector4dF result = lhs;
  result.Add(rhs);
  return result;
}

inline Vector4dF operator-(const Vector4dF& lhs, const Vector4dF& rhs) {
  Vector4dF result = lhs;
  result.Add(-rhs);
  return result;
}

inline Vector4dF operator*(const Vector4dF& v, double scale) {
  Vector4dF result = v;
  result.Scale(scale);
  return result;
}

inline Vector4dF operator*(const Vector4dF& lhs, const Vector4dF& rhs) {
  Vector4dF result = lhs;
  result.Scale(rhs.x(), rhs.y(), rhs.z(), rhs.w());
  return result;
}

CR_ALWAYS_INLINE double Sum(const Vector4dF& v) {
    return v.x() + v.y() + v.z() + v.w();
}

// Return the dot product of two vectors.
///CRUI_EXPORT float DotProduct(const Vector4dF& lhs, const Vector4dF& rhs);
///
///// Return a vector that is |v| scaled by the given scale factors along each
///// axis.
///CRUI_EXPORT Vector4dF ScaleVector4d(const Vector4dF& v,
///                                    float x_scale,
///                                    float y_scale,
///                                    float z_scale,
///                                    float w_scale);
///
///// Return a vector that is |v| scaled by the components of |s|
///inline Vector4dF ScaleVector4d(const Vector4dF& v, const Vector4dF& s) {
///  return ScaleVector4d(v, s.x(), s.y(), s.z(), s.w());
///}
///
///// Return a vector that is |v| scaled by the given scale factor.
///inline Vector4dF ScaleVector4d(const Vector4dF& v, float scale) {
///  return ScaleVector4d(v, scale, scale, scale, scale);
///}

}  // namespace gfx
}  // namespace crui

#endif // UI_GFX_GEOMETRY_VECTOR4D_F_H_
