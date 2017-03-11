/* Gears. Draw them. In 3D.
 *
 * Jordan Justen : gears3d is public domain
 */

#include "main.h"

#include <epoxy/gl.h>
#include <SDL.h>
#include <SDL_opengl.h>

static void
set_gl_compat_attributes()
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
}

static void
win_resize(int width, int height)
{
}

static void
set_gl_compat_state()
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
}

static void
gl_compat_draw()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

static void
gl_compat_destruct()
{
}

struct gears_drawer gl21_compat_drawer = {
    .set_window_attributes = set_gl_compat_attributes,
    .set_global_state = set_gl_compat_state,
    .resize = win_resize,
    .draw = gl_compat_draw,
    .destruct = gl_compat_destruct,
};
