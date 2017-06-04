/* Jordan Justen : gears3d is public domain */

#ifndef __winsys_x11r6_h_included__
#define __winsys_x11r6_h_included__

#include <X11/Xlib.h>
#include <stdbool.h>

bool load_x11r6_library(void);
bool create_x11r6_window(void);
bool get_x11r6_dpy_wnd(Display **dpy, Window *wnd);

#define X11R6(fn) x11r6_##fn

#define XOpenDisplay X11R6(XOpenDisplay)
#define XCreateSimpleWindow X11R6(XCreateSimpleWindow)
#define XCreateWindow X11R6(XCreateWindow)
#define XMapWindow X11R6(XMapWindow)
#define XFlush X11R6(XFlush)
#define XChangeProperty X11R6(XChangeProperty)
#define XStoreName X11R6(XStoreName)
#define XEventsQueued X11R6(XEventsQueued)
#define XNextEvent X11R6(XNextEvent)
#define XSelectInput X11R6(XSelectInput)
#define XKeysymToKeycode X11R6(XKeysymToKeycode)
#define XStringToKeysym X11R6(XStringToKeysym)

#define FNSYM(fn, ret, args)                   \
    typedef ret (*PFN_##fn)args;               \
    extern PFN_##fn fn;
#include "winsys_x11r6_fn.h"
#undef FNSYM

#endif
