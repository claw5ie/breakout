#include "Vectors.hpp"

Vec2f &operator+=(Vec2f &x, const Vec2f &y)
{
  x.x += y.x;
  x.y += y.y;

  return x;
}

Vec2f &operator-=(Vec2f &x, const Vec2f &y)
{
  x.x -= y.x;
  x.y -= y.y;

  return x;
}

Vec2f operator*(const Vec2f &x, float scalar)
{
  return { x.x * scalar, x.y * scalar };
}
