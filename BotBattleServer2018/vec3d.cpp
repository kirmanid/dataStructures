#define _USE_MATH_DEFINES
#include "vec3d.h"
#include <cmath>
#include <algorithm>
#include <limits>
#include <iostream>

using namespace std;

bool isLessThanByX(const Vec3d& p1, const Vec3d& p2)
{
    return p1.x < p2.x;
}

bool isLessThanByY(const Vec3d& p1, const Vec3d& p2)
{
    return p1.y < p2.y;
}

bool isLessThanByZ(const Vec3d& p1, const Vec3d& p2)
{
    return p1.z < p2.z;
}

double Vec3d::magnitude()
{
    return sqrt(x*x + y*y + z*z);
}

double Vec3d::magSquared()
{
    return x*x + y*y + z*z;
}

void Vec3d::scale(double s)
{
    x *= s;
    y *= s;
    z *= s;
}

void Vec3d::rotateZ(double radians)
{
    *this = { x * cos(radians) - y * sin(radians), x * sin(radians) + y * cos(radians), z };
}

void Vec3d::rotateX(double radians)
{
    *this = { x, y * cos(radians) - z * sin(radians), y * sin(radians) + z * cos(radians) };
}

void Vec3d::rotateY(double radians)
{
    *this = { x * cos(radians) - z * sin(radians), y, -x * sin(radians) + z * cos(radians) };
}

void Vec3d::translate(Vec3d offset)
{
    x += offset.x;
    y += offset.y;
    z += offset.z;
}

bool Vec3d::equals(const Vec3d& other, double threshold) const
{
    return fabs(x - other.x) <= threshold && fabs(y - other.y) <= threshold && fabs(z - other.z) <= threshold;
}

Vec3d operator-(Vec3d p1, Vec3d p2)
{
    return Vec3d {p1.x - p2.x, p1.y - p2.y , p1.z - p2.z };
}

Vec3d operator+(Vec3d p1, Vec3d p2)
{
    return Vec3d {p1.x + p2.x, p1.y + p2.y, p1.z + p2.z };
}

Vec3d operator*(Vec3d v, double s)
{
    return Vec3d { v.x * s, v.y * s, v.z * s };
}

Vec3d operator*(double s, Vec3d v)
{
    return Vec3d { v.x * s, v.y * s, v.z * s };
}

bool operator== (Vec3d p1, Vec3d p2)
{
    return p1.x == p2.x && p1.y == p2.y && p1.z == p2.z;
}



double dot(const Vec3d& v1, const Vec3d& v2)
{
    return v1.x*v2.x+v1.y*v2.y+v1.z*v2.z;
}

Vec3d cross(const Vec3d& v1, const Vec3d& v2)
{
    return { v1.y*v2.z-v1.z*v2.y, -(v1.x*v2.z - v1.z*v2.x), v1.x*v2.y-v1.y*v2.x };
}

double distToSegmentSquared(Vec3d p, Vec3d p1, Vec3d p2) {
  double epsilon = 1e-20; // FIX!!
  Vec3d v1 = p2 - p1;
  double segLen = v1.magSquared();
  if (segLen < epsilon) {
      return (p1-p).magSquared();
  }
  double t = dot(p - p1, v1) / segLen;
  if (t <= 0) {
      return (p1-p).magSquared();
  }
  if (t >= 1) {
      return (p2-p).magSquared();
  }
  return (p - (v1*t + p1)).magSquared();
}

Vec3d& Vec3d::operator+= (const Vec3d& v)
{
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
}

Vec3d& Vec3d::operator-= (const Vec3d& v)
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
}

Vec3d& Vec3d::operator*= (double s)
{
    x *= s;
    y *= s;
    z *= s;
    return *this;
}

Vec3d& Vec3d::operator/= (double s)
{
    x /= s;
    y /= s;
    z /= s;
    return *this;
}
