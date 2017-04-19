/* Gears. Draw them. In 3D.
 *
 * Jordan Justen : gears3d is public domain
 */

#include "main.h"

#include <epoxy/gl.h>
#include <SDL.h>
#include <SDL_opengl.h>

static struct gears_drawer *drawer = NULL;
static SDL_Window* window;
static SDL_GLContext context;
static int width, height;
static bool done = false;

static bool
handle_event(bool all)
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_KEYUP:
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                done = true;
                break;
            }
            break;
        case SDL_QUIT:
            done = true;
            break;
        case SDL_WINDOWEVENT:
            switch (event.window.event) {
            case SDL_WINDOWEVENT_RESIZED:
                width = event.window.data1;
                height = event.window.data2;
                drawer->resize(width, height);
                break;
            }
        }

        if (!all) {
            break;
        }
    }

    return true;
}

static
bool sdl_start()
{
    const GLubyte *version;

    if (SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        return false;
    }

    drawer->set_window_attributes();

    bool is_gl = false;
    switch (drawer->api_type) {
    case API_NOT_SET:
        assert(drawer->api_type != API_NOT_SET);
        break;
    case API_OPENGL_COMPAT:
    case API_OPENGL_CORE:
    case API_OPENGL_ES2:
        is_gl = true;
        break;
    }

    Uint32 window_create_flags = SDL_WINDOW_RESIZABLE;
    /* window_create_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP; */

    if (is_gl)
        window_create_flags |= SDL_WINDOW_OPENGL;

    window = SDL_CreateWindow("gears3d", SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, 300, 300,
                              window_create_flags);

    if (is_gl) {
        context = SDL_GL_CreateContext(window);
        if (!context) {
            fprintf(stderr, "SDL_GL_CreateContext(): %s\n", SDL_GetError());
            return false;
        }

        int w, h;
        SDL_GL_MakeCurrent(window, context);
        SDL_GL_GetDrawableSize(window, &w, &h);
        glViewport(0, 0, w, h);
        width = w;
        height = h;

        version = glGetString(GL_VERSION);
        assert(version);
        printf("GL Version: %s\n", version);

        if (gears_options.vsync) {
            SDL_GL_SetSwapInterval(1);
        } else if (SDL_GL_SetSwapInterval(-1 /* late swap tearing */) == -1) {
            /* late swap setup failed */
            SDL_GL_SetSwapInterval(0 /* immediate updates */);
        }

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    } else {
        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        width = w;
        height = h;
    }

    return true;
}

static float angle_per_dt = 0.0;

static void
draw(void)
{
    static double t0 = -1.;
    static GLfloat angle = 0.0;
    double dt, t = SDL_GetTicks() / 1000.0;
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
    SDL_GL_SwapWindow(window);
}

int main(int argc, char **argv)
{
    bool ok;

    ok = parse_options(argc, argv);
    if (!ok)
        return EXIT_FAILURE;

    angle_per_dt = (double)gears_options.speed / 180.0 * M_PI;

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
    }

    sdl_start();

    if (drawer->upgrade_drawer)
        drawer = drawer->upgrade_drawer();

    if (drawer->name)
        printf("Drawing implementation: %s\n", drawer->name);

    uint64_t total_frames = 0, start_frame = 0;
    drawer->set_global_state();
    drawer->resize(width, height);
    const Uint32 start_time = SDL_GetTicks();
    Uint32 t1 = start_time, t2;
    while (!done) {
        handle_event(true);
        draw();
        total_frames++;
        t2 = SDL_GetTicks();

        if (gears_options.max_time_ms != 0 &&
            (t2 - start_time) >= gears_options.max_time_ms) {
            done = true;
        }

        if ((t2 - t1) >= 5000 || done) {
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
    return 0;
}
