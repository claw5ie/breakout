#include <algorithm>
#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cassert>

#include <GL/glew.h>
#include <GL/glxew.h>

#include "X11Window.hpp"
#include "Vectors.hpp"
#include "Utils.hpp"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800

struct AABB
{
  Vec2f pos, shape;
};

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
create_breakout ()
{
  static_assert (sizeof (AABB) == 4 * sizeof (float), ":(");

  Breakout game;

  game.block_count = 6 * 5;
  game.blocks = (AABB *)malloc_or_exit (game.block_count * sizeof (AABB));

  game.slab = { { 0.5, -0.5 }, { 0.3, 0.05 } };
  game.slab_vel = { 0.04, 0.0 };

  game.ball = { { 0.0, -0.8 }, { 0.05, 0.05 } };
  game.ball_vel = { 0.02, 0.01 };

  glCreateVertexArrays (1, &game.vertex_array);
  glCreateBuffers (2, (gluint *)game.buffer);

  {
    auto vertex_shader =
      create_shader (GL_VERTEX_SHADER, "shaders/quad.vert");
    auto fragment_shader =
      create_shader (GL_FRAGMENT_SHADER, "shaders/quad.frag");
    game.program =
      create_program (vertex_shader, fragment_shader);

    glDeleteShader (vertex_shader);
    glDeleteShader (fragment_shader);
  }

  glBindVertexArray (game.vertex_array);

  glBindBuffer (GL_ARRAY_BUFFER, game.buffer[0]);
  glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
  glEnableVertexAttribArray (0);

  glBindBuffer (GL_ARRAY_BUFFER, game.buffer[1]);
  glVertexAttribPointer (1, 4, GL_FLOAT, GL_FALSE, 0, (void *)0);
  glEnableVertexAttribArray (1);
  glVertexAttribDivisor (1, 1);

  {
    Vec2f quad[4] = { { 0.0, 0.0 },
                      { 1.0, 0.0 },
                      { 0.0, 1.0 },
                      { 1.0, 1.0 } };

    glBindBuffer (GL_ARRAY_BUFFER, game.buffer[0]);
    glBufferData (GL_ARRAY_BUFFER, sizeof (quad), quad, GL_STATIC_DRAW);
  }

  {
    float x_offset = 0.04, y_offset = 0.04;
    AABB block =
      { { -1 + x_offset, 1 - y_offset },
        { (2 - (6 + 1) * x_offset) / 6, 0.05 } };

    block.pos.y -= block.shape.y;

    for (size_t i = 0; i < game.block_count; )
      {
        game.blocks[i] = block;

        if (++i % 6 != 0)
          block.pos.x += x_offset + block.shape.x;
        else
          {
            block.pos.x = -1 + x_offset;
            block.pos.y -= y_offset + block.shape.y;
          }
      }
  }

  glBindBuffer (GL_ARRAY_BUFFER, game.buffer[1]);
  glBufferData (GL_ARRAY_BUFFER,
                (game.block_count + 2) * sizeof (AABB),
                NULL,
                GL_DYNAMIC_DRAW);

  glBufferSubData (GL_ARRAY_BUFFER,
                   Breakout::slab_offset,
                   sizeof (AABB),
                   &game.slab);

  glBufferSubData (GL_ARRAY_BUFFER,
                   Breakout::ball_offset,
                   sizeof (AABB),
                   &game.ball);

  glBufferSubData (GL_ARRAY_BUFFER,
                   Breakout::block_offset,
                   game.block_count * sizeof (AABB),
                   game.blocks);

  return game;
}

bool
do_intersect (const AABB &x, const AABB &y)
{
  return !(x.pos.x + x.shape.x < y.pos.x
           || x.pos.x > y.pos.x + y.shape.x
           || x.pos.y + x.shape.y < y.pos.y
           || x.pos.y > y.pos.y + y.shape.y);
}

enum Direction
  {
   Left, Right, Down, Up
  };

// Interpret "pos" as point and "shape" as vector (direction of the line).
Vec2f
parametric_intersect (const AABB &aabb_as_line1,
                      const AABB &aabb_as_line2);

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

void
resolve_collisions (Breakout &game)
{
  for (size_t i = 0; i < game.block_count; i++)
    {
      AABB block = game.blocks[i];

      if (do_intersect (game.ball, block))
        {
          switch (hit_direction (block, game.ball, game.ball_vel))
            {
            case Left:
            case Right:
              game.ball_vel.x = -game.ball_vel.x;
              break;
            case Down:
            case Up:
              game.ball_vel.y = -game.ball_vel.y;
              break;
            }

          break;
        }
    }

  if (do_intersect (game.ball, game.slab))
    {
      switch (hit_direction (game.slab, game.ball, game.ball_vel))
        {
        case Left:
        case Right:
          game.ball_vel.x = -game.ball_vel.x;
          break;
        case Down:
        case Up:
          game.ball_vel.y = -game.ball_vel.y;
          break;
        }
    }
}

void
update (Breakout &game)
{
  if (game.ball.pos.x <= -1
      || game.ball.pos.x + game.ball.shape.x >= 1)
    game.ball_vel.x = -game.ball_vel.x;
  else if (game.ball.pos.y <= -1
           || game.ball.pos.y + game.ball.shape.y >= 1)
    game.ball_vel.y = -game.ball_vel.y;

  resolve_collisions (game);

  game.ball.pos += game.ball_vel;

  glBindBuffer (GL_ARRAY_BUFFER, game.buffer[1]);
  glBufferSubData (GL_ARRAY_BUFFER,
                   Breakout::ball_offset,
                   sizeof (AABB),
                   &game.ball);
}

void
draw (const Breakout &game)
{
  glBindVertexArray (game.vertex_array);
  glUseProgram (game.program);
  glDrawArraysInstanced (GL_TRIANGLE_STRIP, 0, 4, 2 + game.block_count);
}

int
main (void)
{
  X11Window window =
    create_x11_window (SCREEN_WIDTH, SCREEN_HEIGHT, "Hello World!");

  if (glewInit () != GLEW_OK)
    {
      std::fputs ("ERROR: failed to initialize glew.\n", stderr);
      std::exit (EXIT_FAILURE);
    }

  Breakout game = create_breakout ();

  keyboard_context = (void *)&game;
  keyboard_callback =
    [](X11Window &window, KeySym keysym, void *data) -> void
    {
      Breakout &game = *(Breakout *)data;

      bool should_update = false;

      if (keysym == XK_a)
        {
          game.slab.pos -= game.slab_vel;
          should_update = true;
        }
      else if (keysym == XK_d)
        {
          game.slab.pos += game.slab_vel;
          should_update = true;
        }

      if (should_update)
        {
          glBindBuffer (GL_ARRAY_BUFFER, game.buffer[1]);
          glBufferSubData (GL_ARRAY_BUFFER,
                           Breakout::slab_offset,
                           sizeof (AABB),
                           &game.slab);
        }

      window.should_close = (keysym == XK_Escape);
    };

  glClearColor (0.4, 0.4, 0.4, 1.0);

  for (glenum error; (error = glGetError ()) != GL_NO_ERROR; )
    {
      std::fprintf (stderr, "ERROR: detected OpenGL error: %i.\n", error);
    }

  if (GLX_EXT_swap_control)
    glXSwapIntervalEXT (window.display, window.handle, 1);

  while (!window.should_close)
    {
      glClear (GL_COLOR_BUFFER_BIT);

      update (game);
      draw (game);

      glXSwapBuffers (window.display, window.handle);

      process_events (window);
    }

  close (window);
}
