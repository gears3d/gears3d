/* Jordan Justen : gears3d is public domain */

#ifndef __winsys_wl_h_included__
#define __winsys_wl_h_included__

#include <wayland-client.h>
#include <wayland-egl.h>
#include <stdbool.h>

struct wl_display;
struct wl_surface;

bool load_wl_library(void);
bool create_wl_window(void);
bool get_wl_dpy_srf(struct wl_display **rdpy, struct wl_surface **rsrf);

#endif
