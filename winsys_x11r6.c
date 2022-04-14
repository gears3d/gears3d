/* Jordan Justen : gears3d is public domain */

#include "winsys_x11r6.h"
#include "main.h"
#include "load_lib.h"
#include "glx.h"
#include "sim.h"

#include <assert.h>

#define FNSYM(fn, ret, args) PFN_##fn fn = NULL;
#include "winsys_x11r6_fn.h"
#undef FNSYM

bool
load_x11r6_library(void)
{
    static bool tried = false;
    static void *lib = NULL;

    if (tried)
        return lib != NULL;

    tried = true;
    lib = open_library("libX11.so.6");
    if (lib == NULL)
        return false;

#define FNSYM(fn, ret, args)                    \
    fn = get_library_symbol(lib, #fn);          \
    assert(fn != NULL);
#include "winsys_x11r6_fn.h"
#undef FNSYM

    int (*init_threads)(void) = get_library_symbol(lib, "XInitThreads");
    assert(init_threads != NULL);
    init_threads();

    return true;
}

static Display *dpy = NULL;
static Window wid = 0;

bool
create_x11r6_window(void)
{
    if (!load_x11r6_library())
        return false;

    dpy = XOpenDisplay(NULL);
    if (dpy == NULL)
        return false;

    drawer->set_window_attributes();

    Window root_wid = DefaultRootWindow(dpy);

    wid = XCreateWindow(dpy, root_wid, 0, 0, sim_width, sim_height, 0, 24,
                        InputOutput, CopyFromParent, 0, NULL);
    XSelectInput(dpy, wid, KeyPress | KeyRelease | StructureNotifyMask);

    XStoreName(dpy, wid, "gears3d");

    XMapWindow(dpy, wid);
    XFlush(dpy);

    return true;
}

bool
get_x11r6_dpy_wnd(Display **rdpy, Window *rwnd)
{
    if (dpy != NULL) {
        *rdpy = dpy;
        *rwnd = wid;
        return true;
    } else {
        return false;
    }
}

static bool
init(void)
{
    if (!load_x11r6_library())
        return false;
    if (api_is_gl() && !load_glx_library())
        return false;
    return true;
}

static int Escape;

static void
handle_events(void)
{
    assert(dpy != NULL);
    XFlush(dpy);
    int pending = XEventsQueued(dpy, QueuedAlready);
    while (pending > 0) {
        XEvent e;
        XNextEvent(dpy, &e);
        switch (e.type) {
        case KeyRelease:
            if (e.xkey.keycode == Escape) {
                sim_done = true;
                return;
            }
            break;
        case ConfigureNotify:
            if (e.xconfigure.width != sim_width ||
                e.xconfigure.height != sim_height) {
                sim_width = e.xconfigure.width;
                sim_height = e.xconfigure.height;
                drawer->resize(sim_width, sim_height);
            }
            break;
        }
        pending--;
    }
}

static bool
create_window()
{
    if (!create_x11r6_window())
        return false;

    Escape = XKeysymToKeycode(dpy, XStringToKeysym("Escape"));

    if (api_is_gl()) {
        assert(dpy != NULL);
        if (!create_glx_context(dpy, wid))
            return false;
    }

    return true;
}

static void
swap_buffers()
{
    assert(dpy != NULL);

    glx_swap_buffers(dpy, wid);
}

struct winsys winsys_x11r6 = {
    .name = "X11R6",
    .init = init,
    .create_window = create_window,
    .swap_buffers = swap_buffers,
    .handle_events = handle_events,
};
