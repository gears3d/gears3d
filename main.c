/* Jordan Justen : gears3d is public domain */

#include "main.h"
#include "sim.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

struct gears_drawer *drawer = NULL;
struct winsys * winsys = NULL;

static float angle_per_dt = 0.0;

static void
draw(void)
{
    static double t0 = -1.;
    static float angle = 0.0;
    double dt, t = get_sim_time_ms() / 1000.0;
    if (t0 < 0.0)
        t0 = t;
    dt = t - t0;
    t0 = t;

    angle += angle_per_dt * dt;
    angle = fmod(angle, 2 * M_PI); /* prevents eventual overflow */

    drawer->update_angle(angle);
    drawer->draw();
}

void
gl_swapbuffers()
{
    winsys->swap_buffers();
}

int main(int argc, char **argv)
{
    bool ok;

    ok = parse_options(argc, argv);
    if (!ok)
        return EXIT_FAILURE;

    angle_per_dt = (double)gears_options.speed / 180.0 * M_PI;

    if (gears_options.sim_time > 0.0f) {
        sim_fixed_step_time = gears_options.sim_time;
        sim_fixed_step = true;
    }

    switch (gears_options.api_type) {
    case API_OPENGL_COMPAT:
        drawer = &gl21_compat_drawer;
        break;
    case API_NOT_SET:
    case API_OPENGL_CORE:
        drawer = &gl32_core_drawer;
        break;
    case API_OPENGL_ES2:
        drawer = &gles20_drawer;
        break;
    case API_VULKAN:
        drawer = &vk10_drawer;
        break;
    }

    switch (gears_options.winsys_type) {
    case WINSYS_WAYLAND:
        winsys = &winsys_wl;
        break;
    case WINSYS_NONE:
        winsys = &winsys_gbm;
        break;
    case WINSYS_AUTO:
    case WINSYS_X11:
        winsys = &winsys_x11r6;
        break;
    }

    if (!winsys->init()) {
        fprintf(stderr, "Failed to initialize winsys (%s)\n", winsys->name);
        return 1;
    }

    if (!winsys->create_window()) {
        fprintf(stderr, "Failed to create window (%s)\n", winsys->name);
        return 1;
    }

    if (winsys->name)
        printf("Window system: %s\n", winsys->name);

    if (!init_output())
        return EXIT_FAILURE;
    init_sim();

    if (drawer->upgrade_drawer)
        drawer = drawer->upgrade_drawer();

    if (drawer->name)
        printf("Drawing implementation: %s\n", drawer->name);

    uint64_t total_frames = 0, start_frame = 0;
    drawer->set_global_state();
    drawer->resize(sim_width, sim_height);
    const uint64_t start_time = get_sim_time_ms();
    uint64_t t1 = start_time, t2;
    while (!sim_done) {
        winsys->handle_events();
        draw();
        total_frames = frame_drawn();
        t2 = get_sim_time_ms();

        if ((gears_options.max_frames != 0 &&
             total_frames >= gears_options.max_frames) ||
            (gears_options.max_time_ms != 0 &&
             (t2 - start_time) >= gears_options.max_time_ms)) {
            sim_done = true;
        }

        if ((t2 - t1) >= 5000 || sim_done) {
            const int frames = total_frames - start_frame;
            const float time = (t2 - t1) / 1000.0;
            const float fps = frames / time;
            const char *fmt =
                (t2 - t1) > 1000 ?
                "%d frames in %.1f seconds = %.3f FPS\n" :
                "%d frames in %.3f seconds = %.3f FPS\n";

            printf(fmt, frames, time, fps);
            t1 = t2;
            start_frame = total_frames;
        }
    }

    drawer->destruct();
    end_output();
    return 0;
}
