#include <cstdio>
#include <cstdlib>
#include "X11Window.hpp"

KeyboardCallback keyboard_callback = NULL;
MouseCallback mouse_callback = NULL;

void *keyboard_context = NULL;
void *mouse_context = NULL;

time_t when_window_was_created = 0;

time_t
gettime ()
{
  struct timespec ts;

  clock_gettime (CLOCK_MONOTONIC, &ts);

  return (ts.tv_sec - when_window_was_created) * 1000
    + ts.tv_nsec / 100000;
}

X11Window
create_x11_window (uint32_t width,
                   uint32_t height,
                   const char *title)
{
  X11Window window;

  window.should_close = false;
  window.display = XOpenDisplay ((char *)0);

  if (window.display == NULL)
    {
      std::fputs ("ERROR: failed to open display.\n", stderr);
      std::exit (EXIT_FAILURE);
    }

  window.screen = DefaultScreen (window.display);

  GLint attributes[] = { GLX_RGBA,
                         GLX_DEPTH_SIZE,
                         24,
                         GLX_DOUBLEBUFFER,
                         None };

  auto *const visual = glXChooseVisual (window.display,
                                        window.screen,
                                        attributes);

  if (visual == NULL)
    {
      std::fputs ("ERROR: couldn't find appropriate visual.\n", stderr);
      std::exit (EXIT_FAILURE);
    }

  auto const root = DefaultRootWindow (window.display);
  XSetWindowAttributes window_attributes;
  window_attributes.colormap = XCreateColormap (window.display,
                                                root,
                                                visual->visual,
                                                AllocNone);
  window_attributes.event_mask =
    KeyPressMask | KeyReleaseMask | ButtonPressMask;
  window.handle = XCreateWindow (window.display,
                                 root,
                                 0,
                                 0,
                                 width,
                                 height,
                                 0,
                                 visual->depth,
                                 InputOutput,
                                 visual->visual,
                                 CWColormap | CWEventMask,
                                 &window_attributes);
  window.context = glXCreateContext (window.display,
                                     visual,
                                     NULL,
                                     GL_TRUE);

  if (window.context == NULL)
    {
      std::fputs ("ERROR: failed to create OpenGL context.\n", stderr);
      std::exit (EXIT_FAILURE);
    }

  glXMakeCurrent (window.display, window.handle, window.context);

  XSetStandardProperties (window.display,
                          window.handle,
                          title,
                          NULL,
                          None,
                          NULL,
                          0,
                          NULL);
  XMapWindow (window.display, window.handle);

  window.wm_delete_message =
    XInternAtom (window.display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols (window.display,
                   window.handle,
                   &window.wm_delete_message,
                   1);

  when_window_was_created = gettime () / 1000;

  return window;
}

void
close (X11Window &window)
{
  glXDestroyContext (window.display, window.context);
  XDestroyWindow (window.display, window.handle);
  XCloseDisplay (window.display);
}

void
process_events (X11Window &window)
{
  static bool was_a_pressed = false;
  static bool was_d_pressed = false;

  bool should_repeat = true;

  int pending = XPending (window.display);

  while (pending-- > 0)
    {
      XEvent xevent;
      XNextEvent (window.display, &xevent);

      switch (xevent.type)
        {
        case KeyPress:
          {
            KeySym key = XLookupKeysym (&xevent.xkey, 0);

            if (key == XK_a)
              {
                was_a_pressed = true;
                should_repeat = false;
              }
            else if (key == XK_d)
              {
                was_d_pressed = true;
                should_repeat = false;
              }

            if (keyboard_callback != NULL)
              keyboard_callback (window, key, keyboard_context);
          }

          break;
        case KeyRelease:
          {
            if (XEventsQueued (window.display, QueuedAfterReading) > 0)
              {
                XEvent next;
                XPeekEvent (window.display, &next);

                if (next.type == KeyPress
                    && next.xkey.window == xevent.xkey.window
                    && next.xkey.keycode == xevent.xkey.keycode
                    && next.xkey.time == xevent.xkey.time)
                  continue;
              }

            KeySym key = XLookupKeysym (&xevent.xkey, 0);

            if (key == XK_a)
              was_a_pressed = false;
            else if (key == XK_d)
              was_d_pressed = false;
          }

          break;
        case ButtonPress:
          if (mouse_callback != NULL)
            mouse_callback (window, xevent.xbutton, mouse_context);
          break;
        case ClientMessage:
          window.should_close =
            ((Atom)xevent.xclient.data.l[0] == window.wm_delete_message);
          break;
        }
    }

  if (should_repeat && (was_a_pressed || was_d_pressed))
    {
      if (keyboard_callback != NULL)
        keyboard_callback (window,
                           was_a_pressed ? XK_a : XK_d,
                           keyboard_context);
    }
}
