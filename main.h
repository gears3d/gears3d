/* Gears. Draw them. In 3D.
 *
 * Jordan Justen : gears3d is public domain
 */

#ifndef __main_h_included__
#define __main_h_included__

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

struct gears_drawer {
    void (*set_window_attributes)(void);
    void (*set_global_state)(void);
    void (*resize)(int width, int height);
    void (*draw)(void);
    void (*destruct)(void);
};

extern struct gears_drawer gl32_core_drawer;
extern struct gears_drawer gl21_compat_drawer;

#endif