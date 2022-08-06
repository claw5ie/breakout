#ifndef BREAKOUT_HPP
#define BREAKOUT_HPP

#include "gl_types.hpp"
#include "AABB.hpp"

struct Breakout
{
  AABB *blocks;
  size_t block_count;

  AABB slab;
  Vec2f slab_vel;

  AABB ball;
  Vec2f ball_vel;

  gluint vertex_array;
  gluint buffer[2];
  gluint program;

  static uint32_t constexpr slab_offset = 0;
  static uint32_t constexpr ball_offset = sizeof (AABB);
  static uint32_t constexpr block_offset = 2 * sizeof (AABB);
};

Breakout
create_breakout (void);

void
update (Breakout &game);

void
draw (const Breakout &game);

#endif // BREAKOUT_HPP
