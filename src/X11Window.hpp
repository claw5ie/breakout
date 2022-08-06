#ifndef X11WINDOW_HPP
#define X11WINDOW_HPP

#include <functional>
#include <X11/Xlib.h>
#include <GL/glx.h>

struct X11Window
{
  Display *display;
  int screen;
  Window handle;
  GLXContext context;

  Atom wm_delete_message;
  bool should_close;
};

// "context" is used to pass additional information to the callback.
// Effectively it's std::function<...> implemented in C style.
typedef void (*KeyboardCallback)(X11Window &window,
                                 KeySym keysym,
                                 void *context);
typedef void (*MouseCallback)(X11Window &window,
                              XButtonEvent &event,
                              void *context);

extern KeyboardCallback keyboard_callback;
extern MouseCallback mouse_callback;

extern void *keyboard_context;
extern void *mouse_conntext;

time_t
gettime ();

X11Window
create_x11_window (uint32_t width,
                   uint32_t height,
                   const char *title);

void
close (X11Window &window);

void
process_events (X11Window &window);

#endif // X11WINDOW_HPP
