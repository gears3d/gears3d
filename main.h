/* Jordan Justen : gears3d is public domain */

#ifndef __main_h_included__
#define __main_h_included__

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

enum api_type {
    API_NOT_SET,
    API_OPENGL_COMPAT,
    API_OPENGL_CORE,
    API_OPENGL_ES2,
};

struct gears_drawer {
    const char *name;
    enum api_type api_type;
    void (*set_window_attributes)(void);
    struct gears_drawer* (*upgrade_drawer)(void);
    void (*set_global_state)(void);
    void (*resize)(int width, int height);
    void (*update_angle)(float angle);
    void (*draw)(void);
    void (*destruct)(void);
};

extern struct gears_drawer gl32_core_drawer;
extern struct gears_drawer gl21_compat_drawer;
extern struct gears_drawer gles20_drawer;
extern struct gears_drawer gles30_drawer;
extern struct gears_drawer vk10_drawer;

struct gears_options {
    enum api_type api_type;
    bool vsync;
    uint64_t max_frames;
    uint64_t max_time_ms;
    uint64_t speed;
};

bool parse_options(int argc, char **argv);
extern struct gears_options gears_options;

void gl_swapbuffers();

#endif
