/* Jordan Justen : gears3d is public domain */

#include "winsys_wl.h"
#include "main.h"
#include "load_lib.h"
#include "egl.h"
#include "sim.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <linux/input.h>

static struct wl_display *dpy = NULL;
static struct wl_compositor *compositor = NULL;
static struct wl_surface *surface;
static struct wl_shell *shell;
static struct wl_shell_surface *shell_surface;
static struct wl_seat *seat;
static struct wl_keyboard *kbd;

static void
kbd_keymap(void *data, struct wl_keyboard *wl_kbd, uint32_t format,
           int32_t fd, uint32_t size)
{
}

static void
kbd_enter(void *data, struct wl_keyboard *wl_kbd, uint32_t serial,
          struct wl_surface *surface, struct wl_array *keys)
{
}

static void
kbd_leave(void *data, struct wl_keyboard *wl_kbd, uint32_t serial,
          struct wl_surface *surface)
{
}

static void
kbd_key(void *data, struct wl_keyboard *wl_kbd, uint32_t serial,
        uint32_t time, uint32_t key, uint32_t state)
{
    if (state == WL_KEYBOARD_KEY_STATE_RELEASED && key == KEY_ESC)
        sim_done = true;
}

static void
kbd_modifiers(void *data, struct wl_keyboard *wl_kbd, uint32_t serial,
              uint32_t mods_depressed, uint32_t mods_latched,
              uint32_t mods_locked, uint32_t group)
{
}

static void
kbd_repeat_info(void *data, struct wl_keyboard *wl_kbd, int32_t rate,
                int32_t delay)
{
}

static const struct wl_keyboard_listener kbd_listener = {
    .keymap = kbd_keymap,
    .enter = kbd_enter,
    .leave = kbd_leave,
    .key = kbd_key,
    .modifiers = kbd_modifiers,
    .repeat_info = kbd_repeat_info,
};

static void
seat_handle_caps(void *data, struct wl_seat *seat,
                 enum wl_seat_capability caps)
{
    if (caps & WL_SEAT_CAPABILITY_KEYBOARD) {
        kbd = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(kbd, &kbd_listener, NULL);
    } else {
        wl_keyboard_destroy(kbd);
        kbd = NULL;
    }
}

static const struct wl_seat_listener seat_listener = {
    seat_handle_caps,
};

static void
reg_handler(void *data, struct wl_registry *reg, uint32_t id, const char *ifc,
            uint32_t ver)
{
    if (strcmp(ifc, "wl_compositor") == 0) {
        compositor = wl_registry_bind(reg, id, &wl_compositor_interface, 1);
    } else if (strcmp(ifc, "wl_shell") == 0) {
        shell = wl_registry_bind(reg, id, &wl_shell_interface, 1);
    } else if (strcmp(ifc, "wl_seat") == 0) {
        seat = wl_registry_bind(reg, id, &wl_seat_interface, 1);
        wl_seat_add_listener(seat, &seat_listener, NULL);
    }
}

static void
reg_remover(void *data, struct wl_registry *reg, uint32_t id)
{
}

static const struct wl_registry_listener reg_listener = {
    reg_handler,
    reg_remover
};


bool
load_wl_library(void)
{
    return true;
}

bool
create_wl_window(void)
{
    if (!load_wl_library())
        return false;

    dpy = wl_display_connect(NULL);
    if (dpy == NULL)
        return false;

    struct wl_registry *reg = wl_display_get_registry(dpy);
    if (!reg)
        return false;

    wl_registry_add_listener(reg, &reg_listener, NULL);
    wl_display_dispatch(dpy);
    wl_display_roundtrip(dpy);

    if (compositor == NULL || shell == NULL)
        return false;

    surface = wl_compositor_create_surface(compositor);
    if (surface == NULL)
        return false;

    shell_surface = wl_shell_get_shell_surface(shell, surface);
    if (shell_surface == NULL)
        return false;

    wl_shell_surface_set_title(shell_surface, "gears3d");

    wl_shell_surface_set_toplevel(shell_surface);

    drawer->set_window_attributes();

    return true;
}

bool
get_wl_dpy_srf(struct wl_display **rdpy, struct wl_surface **rsrf)
{
    if (dpy != NULL && surface != NULL) {
        *rdpy = dpy;
        *rsrf = surface;
        return true;
    } else {
        return false;
    }
}

static bool
init(void)
{
    if (!load_wl_library())
        return false;
    if (api_is_gl() && !load_egl_library())
        return false;
    return true;
}

static void
handle_events(void)
{
    wl_display_roundtrip(dpy);
}

static bool
create_window()
{
    if (!create_wl_window())
        return false;

    if (api_is_gl()) {
        assert(dpy != NULL);
        struct wl_egl_window *wnd =
            wl_egl_window_create(surface, sim_width, sim_height);
        if (wnd == NULL)
            return false;
        if (!create_egl_context(dpy, wnd))
            return false;
    }

    return true;
}

static void
swap_buffers()
{
    assert(dpy != NULL);

    egl_swap_buffers();
}

struct winsys winsys_wl = {
    .name = "Wayland",
    .init = init,
    .create_window = create_window,
    .swap_buffers = swap_buffers,
    .handle_events = handle_events,
};
