#include "AABB.hpp"

bool
do_intersect (const AABB &x, const AABB &y)
{
  return !(x.pos.x + x.shape.x < y.pos.x
           || x.pos.x > y.pos.x + y.shape.x
           || x.pos.y + x.shape.y < y.pos.y
           || x.pos.y > y.pos.y + y.shape.y);
}

// "l1" defines line "l1.pos + k * l1.shape, k in ]-inf, inf[", same for
// "l2". The result satisfies
// "l1.pos + x * l1.shape == l2.pos + y * l2.shape".
Vec2f
parametric_intersect (const AABB &l1, const AABB &l2)
{
  float area = cross (l1.shape, l2.shape);
  Vec2f delta = l2.pos - l1.pos;

  return { cross (delta, l2.shape) / area,
           cross (delta, l1.shape) / area };
}

Direction
hit_direction (const AABB &static_, const AABB &moving_, const Vec2f &vel)
{
  if (moving_.pos.x > static_.pos.x && moving_.pos.y > static_.pos.y)
    {
      float lambda =
        parametric_intersect ({ static_.pos + static_.shape, { 0, -1 } },
                              { moving_.pos, vel }).x;

      return 0 <= lambda && lambda <= static_.shape.y ? Right : Up;
    }
  else
    {
      float lambda =
        parametric_intersect ({ static_.pos, { 0, 1 } },
                              { moving_.pos + moving_.shape, vel }).x;

      return 0 <= lambda && lambda <= static_.shape.y ? Left : Down;
    }
}
