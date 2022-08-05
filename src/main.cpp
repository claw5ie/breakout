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

  game.slab = {{ 0.5, -0.5 }, { 0.3, 0.05 }};
  game.slab_vel = { 0.04, 0.0 };

  game.ball = {{ 0.0, 0.8 }, { 0.05, 0.05 }};
  game.ball_vel = { 0.01, 0.01 };

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
    Vec2f quad[4] = {{ 0.0, 0.0 },
                     { 1.0, 0.0 },
                     { 0.0, 1.0 },
                     { 1.0, 1.0 }};

    glBindBuffer (GL_ARRAY_BUFFER, game.buffer[0]);
    glBufferData (GL_ARRAY_BUFFER, sizeof (quad), quad, GL_STATIC_DRAW);
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

  return game;
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
  glDrawArraysInstanced (GL_TRIANGLE_STRIP, 0, 4, 2);
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

  struct KeyboardContext
  {
    Breakout &game;
    float dt;
  };

  KeyboardContext key_context = { game, 0 };

  keyboard_context = (void *)&key_context;
  keyboard_callback =
    [](X11Window &window, KeySym keysym, void *data) -> void
    {
      KeyboardContext c = *(KeyboardContext *)data;

      bool should_update = false;

      if (keysym == XK_a)
        {
          c.game.slab.pos -= c.game.slab_vel;
          should_update = true;
        }
      else if (keysym == XK_d)
        {
          c.game.slab.pos += c.game.slab_vel;
          should_update = true;
        }

      if (should_update)
        {
          glBindBuffer (GL_ARRAY_BUFFER, c.game.buffer[1]);
          glBufferSubData (GL_ARRAY_BUFFER,
                           Breakout::slab_offset,
                           sizeof (AABB),
                           &c.game.slab);
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

  double dt = 0;

  while (!window.should_close)
    {
      double const start = get_time ();

      glClear (GL_COLOR_BUFFER_BIT);

      update (game);
      draw (game);

      glXSwapBuffers (window.display, window.handle);

      process_events (window);

      dt = get_time () - start;
      key_context.dt = dt;
    }

  close (window);
}
