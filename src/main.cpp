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

  uint32_t vertex_array, vertex_buffer;
  glCreateVertexArrays (1, &vertex_array);
  glCreateBuffers (1, &vertex_buffer);

  {
    Vec2f quad[4] = {{ 0.0, 0.0 },
                     { 1.0, 0.0 },
                     { 0.0, 1.0 },
                     { 1.0, 1.0 }};

    glBindBuffer (GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData (GL_ARRAY_BUFFER, sizeof (quad), quad, GL_STATIC_DRAW);
  }

  glBindVertexArray (vertex_array);

  glBindBuffer (GL_ARRAY_BUFFER, vertex_buffer);
  glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
  glEnableVertexAttribArray (0);

  uint32_t offsets, scales;
  glCreateBuffers (1, &offsets);
  glCreateBuffers (1, &scales);

  glBindBuffer (GL_ARRAY_BUFFER, offsets);
  glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
  glEnableVertexAttribArray (1);
  glVertexAttribDivisor (1, 1);

  glBindBuffer (GL_ARRAY_BUFFER, scales);
  glVertexAttribPointer (2, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
  glEnableVertexAttribArray (2);
  glVertexAttribDivisor (2, 1);

  Vec2f offset[2] = {{ 0.5, -0.5 }, { 0.0, 0.8 }};
  Vec2f scale[2] = {{ 0.3, 0.05 }, { 0.05, 0.05 }};

  glBindBuffer (GL_ARRAY_BUFFER, offsets);
  glBufferData (GL_ARRAY_BUFFER,
                sizeof (offset),
                (Vec2f *)&offset,
                GL_DYNAMIC_DRAW);
  glBindBuffer (GL_ARRAY_BUFFER, scales);
  glBufferData (GL_ARRAY_BUFFER,
                sizeof (scale),
                (Vec2f *)&scale,
                GL_DYNAMIC_DRAW);

  auto const program =
    []() -> gluint
    {
      auto vertex_shader =
        create_shader (GL_VERTEX_SHADER, "shaders/quad.vert");
      auto fragment_shader =
        create_shader (GL_FRAGMENT_SHADER, "shaders/quad.frag");
      auto program = create_program (vertex_shader, fragment_shader);

      glDeleteShader (vertex_shader);
      glDeleteShader (fragment_shader);

      return program;
    } ();

  struct KeyboardContext
  {
    gluint offsets_buffer;
    Vec2f *offset;
    float dt;
  };

  KeyboardContext key_context = { offsets, &offset[0], 0 };

  keyboard_context = (void *)&key_context;
  keyboard_callback =
    [](X11Window &window, KeySym keysym, void *context_ptr) -> void
    {
      KeyboardContext context = *(KeyboardContext *)context_ptr;

      Vec2f velocity = { 0.04, 0.0 };

      if (keysym == XK_a)
        {
          *context.offset -= velocity;

          glBindBuffer (GL_ARRAY_BUFFER, context.offsets_buffer);
          glBufferSubData (GL_ARRAY_BUFFER,
                           0,
                           sizeof (Vec2f),
                           context.offset);
        }
      else if (keysym == XK_d)
        {
          *context.offset += velocity;

          glBindBuffer (GL_ARRAY_BUFFER, context.offsets_buffer);
          glBufferSubData (GL_ARRAY_BUFFER,
                           0,
                           sizeof (Vec2f),
                           context.offset);
        }

      window.should_close = (keysym == XK_Escape);
    };

  glUseProgram (program);

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

      glDrawArraysInstanced (GL_TRIANGLE_STRIP, 0, 4, 2);

      glXSwapBuffers (window.display, window.handle);

      process_events (window);

      dt = get_time () - start;
      key_context.dt = dt;
    }

  close (window);
}
