/* Gears. Draw them. In 3D.
 *
 * Jordan Justen : gears3d is public domain
 */

#include "main.h"

#include <epoxy/gl.h>
#include <SDL.h>
#include <SDL_opengl.h>

static void
set_gl_core_attributes()
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
}

static void
win_resize(int width, int height)
{
}

static void
set_gl_core_state()
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
}

static void
gl_core_draw()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

static void
gl_core_destruct()
{
}

struct gears_drawer gl32_core_drawer = {
    .set_window_attributes = set_gl_core_attributes,
    .set_global_state = set_gl_core_state,
    .resize = win_resize,
    .draw = gl_core_draw,
    .destruct = gl_core_destruct,
};
