/* Jordan Justen : gears3d is public domain */

#ifndef __sdl_fw_h_included__
#define __sdl_fw_h_included__

#include <SDL.h>
#include <SDL_syswm.h>

void set_sdl_window(SDL_Window* window);

#ifdef SDL_VIDEO_DRIVER_X11
extern Display *gears3d_x_dpy;
extern Window gears3d_x_win;
#endif

#endif
