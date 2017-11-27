/* Jordan Justen : gears3d is public domain */

#ifndef __winsys_drm_h_included__
#define __winsys_drm_h_included__

#include <gbm.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <stdbool.h>

bool load_drm_library(void);
bool create_drm_window(void);

#define DRMFN(fn) p_##fn

#define gbm_create_device DRMFN(gbm_create_device)
#define gbm_surface_create DRMFN(gbm_surface_create)

#define DEF_PDRMFN(fn, ret, args)               \
    typedef ret (*PFN_##fn)args;                \
    extern PFN_##fn fn;

DEF_PDRMFN(gbm_create_device, struct gbm_device*, (int))
DEF_PDRMFN(gbm_surface_create, struct gbm_surface*,
           (struct gbm_device*, uint32_t, uint32_t, uint32_t, uint32_t))

#endif
