/* Jordan Justen : gears3d is public domain */

#ifndef __sdl_fw_h_included__
#define __sdl_fw_h_included__

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

void load_sdl_library(void);
void set_sdl_window(SDL_Window* window);

#ifdef SDL_VIDEO_DRIVER_X11
extern Display *gears3d_x_dpy;
extern Window gears3d_x_win;
#endif

#define SDL2FN(fn) p_##fn

#define SDL_CreateWindow SDL2FN(SDL_CreateWindow)
#define SDL_GL_CreateContext SDL2FN(SDL_GL_CreateContext)
#define SDL_GL_GetAttribute SDL2FN(SDL_GL_GetAttribute)
#define SDL_GL_GetDrawableSize SDL2FN(SDL_GL_GetDrawableSize)
#define SDL_GL_MakeCurrent SDL2FN(SDL_GL_MakeCurrent)
#define SDL_GL_SetAttribute SDL2FN(SDL_GL_SetAttribute)
#define SDL_GL_SetSwapInterval SDL2FN(SDL_GL_SetSwapInterval)
#define SDL_GL_SwapWindow SDL2FN(SDL_GL_SwapWindow)
#define SDL_GetError SDL2FN(SDL_GetError)
#define SDL_GetWindowSize SDL2FN(SDL_GetWindowSize)
#define SDL_GetWindowWMInfo SDL2FN(SDL_GetWindowWMInfo)
#define SDL_Init SDL2FN(SDL_Init)
#define SDL_PollEvent SDL2FN(SDL_PollEvent)

#define DEF_PSDL2FN(fn, ret, args)                      \
    typedef ret (SDLCALL *PFN_##fn)args;      \
    extern PFN_##fn fn

DEF_PSDL2FN(SDL_CreateWindow, SDL_Window *, (const char *, int, int, int, int,
                                             Uint32));
DEF_PSDL2FN(SDL_GL_CreateContext, SDL_GLContext, (SDL_Window *));
DEF_PSDL2FN(SDL_GL_GetAttribute, int, (SDL_GLattr, int*));
DEF_PSDL2FN(SDL_GL_GetDrawableSize, void, (SDL_Window*, int*, int*));
DEF_PSDL2FN(SDL_GL_MakeCurrent, int, (SDL_Window *, SDL_GLContext));
DEF_PSDL2FN(SDL_GL_SetAttribute, int, (SDL_GLattr, int));
DEF_PSDL2FN(SDL_GL_SetSwapInterval, int, (int));
DEF_PSDL2FN(SDL_GL_SwapWindow, void, (SDL_Window*));
DEF_PSDL2FN(SDL_GetError, const char *, (void));
DEF_PSDL2FN(SDL_GetWindowSize, void, (SDL_Window*, int*, int*));
DEF_PSDL2FN(SDL_GetWindowWMInfo, SDL_bool, (SDL_Window*, SDL_SysWMinfo*));
DEF_PSDL2FN(SDL_Init, int, (Uint32 flags));
DEF_PSDL2FN(SDL_PollEvent, int, (SDL_Event*));

#endif
