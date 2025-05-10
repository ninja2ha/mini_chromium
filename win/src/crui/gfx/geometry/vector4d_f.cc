// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crui/gfx/geometry/vector4d_f.h"

#include <cmath>

#include "crbase/strings/stringprintf.h"
#include "crui/gfx/geometry/angle_conversions.h"

namespace {
constexpr double kEpsilon = 1.0e-6;
}

namespace crui {
namespace gfx {

double Vector4dF::LengthSquared() const {
  return static_cast<double>(vec_[0]) * vec_[0] + 
         static_cast<double>(vec_[1]) * vec_[1] +
         static_cast<double>(vec_[2]) * vec_[2] + 
         static_cast<double>(vec_[3]) * vec_[3];
}

double Vector4dF::Length() const {
  return std::sqrt(LengthSquared());
}

bool Vector4dF::IsZero() const {
  return vec_[0] == 0 && vec_[1] == 0 && vec_[2] == 0 && vec_[3] == 0;
}

void Vector4dF::Add(const Vector4dF& other) {
  vec_[0] += other.vec_[0];
  vec_[1] += other.vec_[1];
  vec_[2] += other.vec_[2];
  vec_[3] += other.vec_[3];
}

void Vector4dF::Subtract(const Vector4dF& other) {
  vec_[0] -= other.vec_[0];
  vec_[1] -= other.vec_[1];
  vec_[2] -= other.vec_[2];
  vec_[3] -= other.vec_[3];
}

void Vector4dF::Scale(double x_scale, double y_scale,
                      double z_scale, double w_scale) {
  vec_[0] *= x_scale;
  vec_[1] *= y_scale;
  vec_[2] *= z_scale;
  vec_[3] *= w_scale;
}

void Vector4dF::InvScale(double x_scale, double y_scale, 
                         double z_scale, double w_scale) {
  vec_[0] /= x_scale;
  vec_[1] /= y_scale;
  vec_[2] /= z_scale;
  vec_[3] /= w_scale;
}

bool Vector4dF::GetNormalized(Vector4dF* out) const {
  double length_squared = LengthSquared();
  *out = *this;
  if (length_squared < kEpsilon * kEpsilon)
    return false;
  out->InvScale(sqrt(length_squared));
  return true;
}

std::string Vector4dF::ToString() const {
  return cr::StringPrintf("[%f %f %f %f]", vec_[0], vec_[1], vec_[2], vec_[3]);
}

///float DotProduct(const Vector4dF& lhs, const Vector4dF& rhs) {
///  return lhs.x() * rhs.x() + lhs.y() * rhs.y() + 
///         lhs.z() * rhs.z() + lhs.w() * rhs.w();
///}
///
///Vector4dF ScaleVector4d(const Vector4dF& v,
///                        double x_scale,
///                        double y_scale,
///                        double z_scale,
///                        double w_scale) {
///  Vector4dF scaled_v(v);
///  scaled_v.Scale(x_scale, y_scale, z_scale, w_scale);
///  return scaled_v;
///}

}  // namespace gfx
}  // namespace crui
