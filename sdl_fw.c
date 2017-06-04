/* Jordan Justen : gears3d is public domain */

#include "sdl_fw.h"
#include "load_lib.h"

#include <assert.h>

#define DECL_PSDL2FN(fn) PFN_##fn fn = NULL

DECL_PSDL2FN(SDL_CreateWindow);
DECL_PSDL2FN(SDL_GL_CreateContext);
DECL_PSDL2FN(SDL_GL_GetAttribute);
DECL_PSDL2FN(SDL_GL_GetDrawableSize);
DECL_PSDL2FN(SDL_GL_MakeCurrent);
DECL_PSDL2FN(SDL_GL_SetAttribute);
DECL_PSDL2FN(SDL_GL_SetSwapInterval);
DECL_PSDL2FN(SDL_GL_SwapWindow);
DECL_PSDL2FN(SDL_GetError);
DECL_PSDL2FN(SDL_GetWindowSize);
DECL_PSDL2FN(SDL_GetWindowWMInfo);
DECL_PSDL2FN(SDL_Init);
DECL_PSDL2FN(SDL_PollEvent);

void
load_sdl_library(void)
{
#define DLSYM(f) \
    f = get_library_symbol(lib, #f);            \
    assert(f != NULL)

    void *lib = open_library("libSDL2-2.0.so.0");
    if (lib == NULL)
        abort();

    DLSYM(SDL_CreateWindow);
    DLSYM(SDL_GL_CreateContext);
    DLSYM(SDL_GL_GetAttribute);
    DLSYM(SDL_GL_GetDrawableSize);
    DLSYM(SDL_GL_MakeCurrent);
    DLSYM(SDL_GL_SetAttribute);
    DLSYM(SDL_GL_SetSwapInterval);
    DLSYM(SDL_GL_SwapWindow);
    DLSYM(SDL_GetError);
    DLSYM(SDL_GetWindowSize);
    DLSYM(SDL_GetWindowWMInfo);
    DLSYM(SDL_Init);
    DLSYM(SDL_PollEvent);
}

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
