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
    API_VULKAN,
};

enum output_type {
    OUTPUT_NONE,
    OUTPUT_PPM,
};

enum winsys_type {
    WINSYS_AUTO,
    WINSYS_GBM,
    WINSYS_WAYLAND,
    WINSYS_X11,
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

extern struct gears_drawer *drawer;

static inline bool api_is_gl(void)
{
    switch (drawer->api_type) {
    case API_OPENGL_COMPAT:
    case API_OPENGL_CORE:
    case API_OPENGL_ES2:
        return true;
    default:
        return false;
    }
}

struct winsys {
    const char *name;
    bool (*init)(void);
    bool (*create_window)();
    void (*swap_buffers)();
    void (*handle_events)();
};

extern struct winsys winsys_drm;
extern struct winsys winsys_gbm;
extern struct winsys winsys_wl;
extern struct winsys winsys_x11r6;

extern struct winsys *winsys;

struct gears_options {
    enum api_type api_type;
    enum winsys_type winsys_type;
    bool vsync;
    uint64_t max_frames;
    uint64_t max_time_ms;
    float sim_time;
    uint64_t speed;
    uint64_t win_width;
    uint64_t win_height;
    enum output_type output_type;
    const char *output_file;
    int extension_offset;
};

bool parse_options(int argc, char **argv);
extern struct gears_options gears_options;

void gl_swapbuffers();

bool init_output(void);
void end_output(void);
bool ready_for_new_frame(void);
void new_frame_data(const void *rgba8);

bool start_gif(const char *filename);
void add_gif_frame(const void *rgba8, uint64_t delay_cs);
void end_gif(void);

#endif
