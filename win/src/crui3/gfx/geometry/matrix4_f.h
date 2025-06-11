// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_GEOMETRY_MATRIX44_H_
#define UI_GFX_GEOMETRY_MATRIX44_H_

#include "crbase/logging.h"
#include "crbase/containers/span.h"
#include "crbase/containers/optional.h"
#include "crbase/compiler_specific.h"
#include "crui/base/ui_export.h"
#include "crui/gfx/geometry/vector4d_f.h"

namespace crui {
namespace gfx {

struct DecomposedTransform;

// This is the underlying data structure of Transform. Don't use this type
// directly.
//
// Throughout this class, we will be speaking in column vector convention.
// i.e. Applying a transform T to vector V is T * V.
// The components of the matrix and the vector look like:
//    \  col
// r   \     0        1        2        3
// o  0 | scale_x  skew_xy  skew_xz  trans_x |   | x |
// w  1 | skew_yx  scale_y  skew_yz  trans_y | * | y |
//    2 | skew_zx  skew_zy  scale_z  trans_z |   | z |
//    3 | persp_x  persp_y  persp_z  persp_w |   | w |
//
// Note that the names are just for remembering and don't have the exact
// meanings when other components exist.
//
// The components correspond to the DOMMatrix mij (i,j = 1..4) components:
//   i = col + 1
//   j = row + 1
class CRUI_EXPORT Matrix4F {
 public:
  enum UninitializedTag { kUninitialized };
  explicit Matrix4F(UninitializedTag) {}

  constexpr Matrix4F()
      : matrix_{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}} {}

  // The parameters are in col-major order.
  constexpr Matrix4F(double r0c0, double r1c0, double r2c0, double r3c0,
                     double r0c1, double r1c1, double r2c1, double r3c1,
                     double r0c2, double r1c2, double r2c2, double r3c2,
                     double r0c3, double r1c3, double r2c3, double r3c3)
      // matrix_ is indexed by [col][row] (i.e. col-major).
      : matrix_{{r0c0, r1c0, r2c0, r3c0},
                {r0c1, r1c1, r2c1, r3c1},
                {r0c2, r1c2, r2c2, r3c2},
                {r0c3, r1c3, r2c3, r3c3}} {}

  bool operator==(const Matrix4F& other) const {
    return Col(0) == other.Col(0) && Col(1) == other.Col(1) &&
           Col(2) == other.Col(2) && Col(3) == other.Col(3);
  }
  bool operator!=(const Matrix4F& other) const { return !(other == *this); }

  // Returns true if the matrix is identity.
  bool IsIdentity() const { return *this == Matrix4F(); }

  // Returns true if the matrix contains translate or is identity.
  bool IsIdentityOrTranslation() const {
    return Col(0) == Vector4dF(1, 0, 0, 0) &&
           Col(1) == Vector4dF(0, 1, 0, 0) &&
           Col(2) == Vector4dF(0, 0, 1, 0) && matrix_[3][3] == 1;
  }

  // Returns true if the matrix only contains scale or translate or is identity.
  bool IsScaleOrTranslation() const {
    return Vector4dF(matrix_[0][1], matrix_[0][2], matrix_[0][3],
                     matrix_[1][0]) == Vector4dF(0, 0, 0, 0) &&
           Vector4dF(matrix_[1][2], matrix_[1][3], matrix_[2][0],
                     matrix_[2][1]) == Vector4dF(0, 0, 0, 0) &&
           matrix_[2][3] == 0 && matrix_[3][3] == 1;
  }

  // Returns true if the matrix only contains scale or is identity.
  bool IsScale() const {
    return IsScaleOrTranslation() && Col(3) == Vector4dF(0, 0, 0, 1);
  }

  bool IsFlat() const {
    return Col(2) == Vector4dF(0, 0, 1, 0) &&
           Vector4dF(matrix_[0][2], matrix_[1][2], 0, matrix_[3][2]) ==
                     Vector4dF(0, 0, 0, 0);
  }

  bool HasPerspective() const {
    return !(Vector4dF(matrix_[0][3], matrix_[1][3], matrix_[2][3],
                       matrix_[3][3]) == Vector4dF(0, 0, 0, 1));
  }

  bool Is2dTransform() const { return IsFlat() && !HasPerspective(); }

  // Gets a value at |row|, |col| from the matrix.
  double rc(int row, int col) const {
    CR_DCHECK(static_cast<unsigned>(row) <= 3u);
    CR_DCHECK(static_cast<unsigned>(col) <= 3u);
    return matrix_[col][row];
  }

  // Set a value in the matrix at |row|, |col|.
  void set_rc(int row, int col, double value) {
    CR_DCHECK(static_cast<unsigned>(row) <= 3u);
    CR_DCHECK(static_cast<unsigned>(col) <= 3u);
    matrix_[col][row] = value;
  }

  void GetColMajor(double[16]) const;
  void GetColMajorF(float[16]) const;

  // this = this * translation.
  void PreTranslate(double dx, double dy);
  void PreTranslate3d(double dx, double dy, double dz);
  // this = translation * this.
  void PostTranslate(double dx, double dy);
  void PostTranslate3d(double dx, double dy, double dz);

  // this = this * scale.
  void PreScale(double sx, double sy);
  void PreScale3d(double sx, double sy, double sz);
  // this = scale * this.
  void PostScale(double sx, double sy);
  void PostScale3d(double sx, double sy, double sz);

  // Rotates this matrix about the specified unit-length axis vector,
  // by an angle specified by its sin() and cos(). This does not attempt to
  // verify that axis(x, y, z).length() == 1 or that the sin, cos values are
  // correct. this = this * rotation.
  void RotateUnitSinCos(double x,
                        double y,
                        double z,
                        double sin_angle,
                        double cos_angle);

  // Special case for x, y or z axis of the above function.
  void RotateAboutXAxisSinCos(double sin_angle, double cos_angle);
  void RotateAboutYAxisSinCos(double sin_angle, double cos_angle);
  void RotateAboutZAxisSinCos(double sin_angle, double cos_angle);

  // this = this * skew.
  void Skew(double tan_skew_x, double tan_skew_y);

  //               |1 skew[0] skew[1] 0|
  // this = this * |0    1    skew[2] 0|
  //               |0    0      1     0|
  //               |0    0      0     1|
  void ApplyDecomposedSkews(cr::Span<const double, 3> skews);

  // this = this * perspective.
  void ApplyPerspectiveDepth(double perspective);

  // this = this * m.
  void PreConcat(const Matrix4F& m) { SetConcat(*this, m); }
  // this = m * this.
  void PostConcat(const Matrix4F& m) { SetConcat(m, *this); }
  // this = a * b.
  void SetConcat(const Matrix4F& a, const Matrix4F& b);

  // Returns true and set |inverse| to the inverted matrix if this matrix
  // is invertible. Otherwise return false and leave the |inverse| parameter
  // unchanged.
  bool GetInverse(Matrix4F& inverse) const;

  bool IsInvertible() const;
  double Determinant() const;

  // Transposes this matrix in place.
  void Transpose();

  // See Transform::Zoom().
  void Zoom(float zoom_factor);

  // Applies the matrix to the vector in place.
  void MapVector4(Vector4dF& vec) const;

  // Same as above, but assumes the vec.y() is 0 and vec.w() is 1, discards
  // vec.z(), and returns vec.w().
  double MapVector2(double vec[2]) const;

  void Flatten();

  cr::Optional<DecomposedTransform> Decompose() const;

 private:
  cr::Optional<DecomposedTransform> Decompose2d() const;

  CR_ALWAYS_INLINE Vector4dF Col(int i) const { return Vector4dF(matrix_[i]); }
  CR_ALWAYS_INLINE void SetCol(int i, const Vector4dF& v) { 
    matrix_[i][0] = v.x();
    matrix_[i][1] = v.y();
    matrix_[i][2] = v.z();
    matrix_[i][3] = v.w();
  }

  // This is indexed by [col][row].
  double matrix_[4][4];
};

}  // namespace gfx
}  // namespace crui

#endif  // UI_GFX_GEOMETRY_MATRIX44_H_