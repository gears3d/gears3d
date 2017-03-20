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
extern struct gears_drawer gles30_drawer;

struct gears_options {
    bool compat;
    bool core;
    bool es;
    bool vsync;
    uint64_t max_time_ms;
};

bool parse_options(int argc, char **argv);
extern struct gears_options gears_options;

#endif
