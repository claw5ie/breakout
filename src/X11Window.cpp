#include <cstdio>
#include <cstdlib>
#include "X11Window.hpp"

KeyboardCallback keyboard_callback = NULL;
MouseCallback mouse_callback = NULL;

void *keyboard_context = NULL;
void *mouse_context = NULL;

uint32_t start;

double
get_time ()
{
  struct timespec ts;

  if (timespec_get (&ts, TIME_UTC) == 0)
    {
      std::fputs ("ERROR: failed to retrieve time.\n", stderr);
      std::exit (EXIT_FAILURE);
    }

  return (double)(ts.tv_sec - start) + ts.tv_nsec * 1e-9;
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
  window.event_masks = KeyPressMask | ButtonPressMask;
  window_attributes.event_mask = window.event_masks;
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

  struct timespec ts;

  if (timespec_get (&ts, TIME_UTC) == 0)
    {
      std::fputs ("ERROR: failed to initialize time.\n", stderr);
      std::exit (EXIT_FAILURE);
    }

  start = ts.tv_sec;

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
  XEvent xevent;

  while (XCheckWindowEvent (window.display,
                            window.handle,
                            window.event_masks,
                            &xevent) ||
         XCheckTypedWindowEvent (window.display,
                                 window.handle,
                                 ClientMessage,
                                 &xevent))
    {
      switch (xevent.type)
        {
        case KeyPress:
          if (keyboard_callback != NULL)
            keyboard_callback (window,
                               XLookupKeysym (&xevent.xkey, 0),
                               keyboard_context);
          break;
        case ButtonPress:
          if (mouse_callback != NULL)
            mouse_callback (window,
                            xevent.xbutton,
                            mouse_context);
          break;
        case ClientMessage:
          window.should_close =
            ((Atom)xevent.xclient.data.l[0] == window.wm_delete_message);
          break;
        }
    }
}
