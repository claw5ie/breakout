#ifndef VECTORS_HPP
#define VECTORS_HPP

struct Vec2f
{
  float x, y;
};

Vec2f &operator+=(Vec2f &x, const Vec2f &y);

Vec2f &operator-=(Vec2f &x, const Vec2f &y);

Vec2f operator*(const Vec2f &x, float scalar);

#endif // VECTORS_HPP
