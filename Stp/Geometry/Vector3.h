// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_GEOMETRY_VECTOR3_H_
#define STP_BASE_GEOMETRY_VECTOR3_H_

#include "Geometry/Vector2.h"

namespace stp {

struct Vector3;
class Xform3;

struct BASE_EXPORT IntVector3 {
  IntVector3() : x(0), y(0), z(0) {}
  IntVector3(int x, int y, int z) : x(x), y(y), z(z) {}

  explicit IntVector3(const IntVector2& o, int z = 0) : x(o.x), y(o.y), z(z) {}

  bool IsZero() const { return x == 0 && y == 0 && z == 0; }

  int64_t GetLengthSquared() const;

  double GetLength() const;

  IntVector3 operator-() const { return IntVector3(-x, -y, -z); }

  IntVector2 As2() const { return IntVector2(x, y); }

  void operator+=(const IntVector3& other);
  void operator-=(const IntVector3& other);

  IntVector3 operator+(const IntVector3& rhs) const { IntVector3 r = *this; r += rhs; return r; }
  IntVector3 operator-(const IntVector3& rhs) const { IntVector3 r = *this; r -= rhs; return r; }

  IntVector3 operator*(int f) const { return IntVector3(x * f, y * f, z * f); }
  IntVector3 operator/(int f) const { return IntVector3(x / f, y / f, z / f); }

  Vector3 operator*(float factor) const;
  Vector3 operator/(float factor) const;

  bool operator==(const IntVector3& o) const { return x == o.x && y == o.y && z == o.z; }
  bool operator!=(const IntVector3& o) const { return !operator==(o); }

  int x;
  int y;
  int z;
};
typedef IntVector3 IntPoint3;

struct BASE_EXPORT Vector3 {
  constexpr Vector3() : x(0), y(0), z(0) {}
  constexpr Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

  explicit Vector3(const Vector2& o, float z = 0)
      : x(o.x), y(o.y), z(z) {}

  bool IsZero() const { return x == 0 && y == 0 && z == 0; }

  double GetLengthSquared() const;

  double GetLength() const;

  void Scale(float x_scale, float y_scale, float z_scale);
  Vector3 GetScaled(float x_scale, float y_scale, float z_scale) const;

  // Scale the vector to have the specified length. If the original length is
  // degenerately small (nearly zero), set it to zero and return false,
  // otherwise return true.
  bool TryScaleToLength(double length);

  // Normalizes the vector. Intentionally does not check whether vector is
  // already normalized. If vector's length is zero, nothing is changed and
  // false is returned.
  bool TryNormalize() WARN_UNUSED_RESULT;
  Vector3 GetNormalizedOrThis() const WARN_UNUSED_RESULT;

  bool IsNormalized() const;

  Vector2 As2() const { return Vector2(x, y); }

  Vector3 operator-() const { return Vector3(-x, -y, -z); }

  void operator+=(const Vector3& other);
  void operator-=(const Vector3& other);

  Vector3 operator+(const Vector3& rhs) const { Vector3 r = *this; r += rhs; return r; }
  Vector3 operator-(const Vector3& rhs) const { Vector3 r = *this; r -= rhs; return r; }

  void operator*=(float scale) { Scale(scale, scale, scale); }

  Vector3 operator*(float f) const { return Vector3(x * f, y * f, z * f); }
  Vector3 operator/(float f) const { return Vector3(x / f, y / f, z / f); }

  bool operator==(const Vector3& o) const { return x == o.x && y == o.y && z == o.z; }
  bool operator!=(const Vector3& o) const { return !operator==(o); }

  float x;
  float y;
  float z;
};
typedef Vector3 Point3;

inline IntVector3 mathAbs(const IntVector3& v) { return IntVector3(mathAbs(v.x), mathAbs(v.y), mathAbs(v.z)); }
inline Vector3 mathAbs(const Vector3& v) { return Vector3(mathAbs(v.x), mathAbs(v.y), mathAbs(v.z)); }

BASE_EXPORT IntVector3 min(const IntVector3& lhs, const IntVector3& rhs);
BASE_EXPORT IntVector3 max(const IntVector3& lhs, const IntVector3& rhs);

BASE_EXPORT Vector3 min(const Vector3& lhs, const Vector3& rhs);
BASE_EXPORT Vector3 max(const Vector3& lhs, const Vector3& rhs);

BASE_EXPORT IntVector3 lerp(const IntVector3& a, const IntVector3& b, double t);

BASE_EXPORT Vector3 lerp(const Vector3& a, const Vector3& b, double t);

BASE_EXPORT bool isNear(const Vector3& lhs, const Vector3& rhs, float tolerance);

BASE_EXPORT Vector3 mathFloor(const Vector3& v);
BASE_EXPORT Vector3 mathCeil(const Vector3& v);
BASE_EXPORT Vector3 mathTrunc(const Vector3& v);
BASE_EXPORT Vector3 mathRound(const Vector3& v);

BASE_EXPORT IntVector3 mathFloorToInt(const Vector3& v);
BASE_EXPORT IntVector3 mathCeilToInt(const Vector3& v);
BASE_EXPORT IntVector3 mathTruncToInt(const Vector3& v);
BASE_EXPORT IntVector3 mathRoundToInt(const Vector3& v);

BASE_EXPORT Vector3 CrossProduct(const Vector3& lhs, const Vector3& rhs);

inline float DotProduct(const Vector3& lhs, const Vector3& rhs) {
  return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

inline Vector3 Vector3::GetScaled(float x_scale, float y_scale, float z_scale) const {
  Vector3 r = *this;
  r.Scale(x_scale, y_scale, z_scale);
  return r;
}

inline void IntVector3::operator+=(const IntVector3& other) {
  x += other.x;
  y += other.y;
  z += other.z;
}

inline void IntVector3::operator-=(const IntVector3& other) {
  x -= other.x;
  y -= other.y;
  z -= other.z;
}

inline void Vector3::operator+=(const Vector3& other) {
  x += other.x;
  y += other.y;
  z += other.z;
}
inline void Vector3::operator-=(const Vector3& other) {
  x -= other.x;
  y -= other.y;
  z -= other.z;
}

inline void Vector3::Scale(float x_scale, float y_scale, float z_scale) {
  x *= x_scale;
  y *= y_scale;
  z *= z_scale;
}

inline Vector3 IntVector3::operator*(float factor) const {
  return Vector3(x * factor, y * factor, z * factor);
}
inline Vector3 IntVector3::operator/(float factor) const {
  return Vector3(x / factor, y / factor, z / factor);
}

} // namespace stp

#endif // STP_BASE_GEOMETRY_VECTOR3_H_
