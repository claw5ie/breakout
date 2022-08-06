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
#include "AABB.hpp"
#include "Breakout.hpp"
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

  Breakout game = create_breakout ();

  keyboard_context = (void *)&game;
  keyboard_callback =
    [](X11Window &window, KeySym keysym, void *data) -> void
    {
      Breakout &game = *(Breakout *)data;

      bool should_update = false;

      if (keysym == XK_a && game.slab.pos.x > -1)
        {
          game.slab.pos -= game.slab_vel;
          should_update = true;
        }
      else if (keysym == XK_d && game.slab.pos.x + game.slab.shape.x < 1)
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
