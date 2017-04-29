/* Jordan Justen : gears3d is public domain */

#include "sdl_fw.h"
#include <SDL_syswm.h>

#include <assert.h>

static SDL_Window* gears3d_sdl_window = NULL;

#ifdef SDL_VIDEO_DRIVER_X11
Display *gears3d_x_dpy = NULL;
Window gears3d_x_win = None;
#endif

void
set_sdl_window(SDL_Window* window)
{
    gears3d_sdl_window = window;

    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(window, &info);
    switch (info.subsystem) {
#ifdef SDL_VIDEO_DRIVER_X11
    case SDL_SYSWM_X11:
        gears3d_x_dpy = info.info.x11.display;
        gears3d_x_win = info.info.x11.window;
        break;
#endif
    default:
        assert(!"Unsupported SDL subsystem!");
    }
}
