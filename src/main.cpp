#include <algorithm>
#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cassert>

#include <GL/glew.h>

#include "X11Window.hpp"

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

  keyboard_callback =
    [](X11Window &window, KeySym keysym, void *) -> void
    {
      window.should_close = (keysym == XK_Escape);
    };

  glClearColor (0.4, 0.4, 0.4, 1.0);

  while (!window.should_close)
    {
      glClear (GL_COLOR_BUFFER_BIT);

      glXSwapBuffers (window.display, window.handle);

      process_events (window);
    }

  close (window);
}
