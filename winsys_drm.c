/* Jordan Justen : gears3d is public domain */

#include "winsys_drm.h"
#include "main.h"
#include "load_lib.h"
#include "egl.h"
#include "sim.h"

#include <assert.h>
#include <fcntl.h>
#include <gbm.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>

#define DECL_PDRMFN(fn) PFN_##fn fn = NULL

DECL_PDRMFN(gbm_create_device);
DECL_PDRMFN(gbm_surface_create);

static struct gbm {
    int fd;
    struct gbm_device *dev;
    struct gbm_surface *surf;
} gbm = { 0, };

bool
load_drm_library(void)
{
    static bool tried = false;
    static void *libgbm = NULL;

    if (tried)
        return libgbm != NULL;

    tried = true;

    #define DLSYM(f)                            \
        f = get_library_symbol(libgbm, #f);     \
        assert(f != NULL)

    libgbm = open_library("libgbm.so.1");
    if (libgbm == NULL)
        return false;

    DLSYM(gbm_create_device);
    DLSYM(gbm_surface_create);

    #undef DLSYM

    return true;
}

bool
create_drm_window(void)
{
    gbm.fd = open("/dev/dri/renderD128", O_RDWR);
    if (gbm.fd < 0)
        return false;
    gbm.dev = gbm_create_device(gbm.fd);
    if (gbm.dev == NULL) {
        printf("Failed to create gbm device!\n");
        return false;
    }
    gbm.surf = gbm_surface_create(gbm.dev, sim_width, sim_height,
                                  GBM_FORMAT_XRGB8888, GBM_BO_USE_RENDERING);
    if (gbm.surf == NULL) {
        printf("Failed to create gbm surface!\n");
        return false;
    }

    drawer->set_window_attributes();

    return true;
}

static bool
init(void)
{
    if (!load_drm_library())
        return false;
    if (api_is_gl() && !load_egl_library())
        return false;
    return true;
}

static void
handle_events(void)
{
}

static bool
create_window()
{
    if (!create_drm_window())
        return false;

    if (api_is_gl()) {
        if (gbm.surf == NULL)
            return false;
        if (!create_egl_context((EGLNativeDisplayType)gbm.dev,
                                (EGLNativeWindowType)gbm.surf)) {
            printf("Failed to create EGL context\n");
            return false;
        }
    }

    return true;
}

static void
swap_buffers()
{
    assert(gbm.dev != NULL);
    egl_swap_buffers();
}

struct winsys winsys_gbm = {
    .name = "GBM",
    .init = init,
    .create_window = create_window,
    .swap_buffers = swap_buffers,
    .handle_events = handle_events,
};
