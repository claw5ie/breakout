#ifndef AABB_HPP
#define AABB_HPP

#include "Vectors.hpp"

enum Direction
  {
   Left, Right, Down, Up
  };

struct AABB
{
  Vec2f pos, shape;
};

bool
do_intersect (const AABB &x, const AABB &y);

Vec2f
parametric_intersect (const AABB &aabb_as_line1,
                      const AABB &aabb_as_line2);

Direction
hit_direction (const AABB &static_, const AABB &moving_, const Vec2f &vel);

#endif // AABB_HPP
