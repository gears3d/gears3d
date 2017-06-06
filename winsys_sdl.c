/* Jordan Justen : gears3d is public domain */

#include "main.h"

#include <epoxy/gl.h>
#include "sdl_fw.h"
#include "sim.h"
#include <SDL2/SDL_opengl.h>

static SDL_Window* window;
static SDL_GLContext context;
static int width, height;

static bool
init(void)
{
    load_sdl_library();

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

static void
handle_events(void)
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_KEYUP:
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                sim_done = true;
                break;
            }
            break;
        case SDL_QUIT:
            sim_done = true;
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
    }
}

static bool
create_window()
{
    const GLubyte *version;

    drawer->set_window_attributes();

    bool is_gl = api_is_gl();

    Uint32 window_create_flags = SDL_WINDOW_RESIZABLE;
    /* window_create_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP; */

    if (is_gl)
        window_create_flags |= SDL_WINDOW_OPENGL;

    window = SDL_CreateWindow("gears3d", SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, sim_width, sim_height,
                              window_create_flags);
    set_sdl_window(window);

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
    } else {
        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        width = w;
        height = h;
    }

    return true;
}

static void
swap_buffers()
{
    SDL_GL_SwapWindow(window);
}

struct winsys winsys_sdl = {
    .name = "SDL2",
    .init = init,
    .create_window = create_window,
    .swap_buffers = swap_buffers,
    .handle_events = handle_events,
};
