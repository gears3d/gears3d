/* Jordan Justen : gears3d is public domain */

#include "glx.h"
#include "load_lib.h"
#include "main.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define DECL_PGLXFN(fn) PFN_##fn fn = NULL

DECL_PGLXFN(glXChooseFBConfig);
DECL_PGLXFN(glXChooseVisual);
DECL_PGLXFN(glXCreateContext);
DECL_PGLXFN(glXCreateContextAttribsARB);
DECL_PGLXFN(glXDestroyContext);
DECL_PGLXFN(glXGetProcAddress);
DECL_PGLXFN(glXGetSwapIntervalMESA);
DECL_PGLXFN(glXMakeCurrent);
DECL_PGLXFN(glXQueryExtension);
DECL_PGLXFN(glXQueryExtensionsString);
DECL_PGLXFN(glXSwapBuffers);
DECL_PGLXFN(glXSwapIntervalEXT);
DECL_PGLXFN(glXSwapIntervalMESA);

bool
load_glx_library(void)
{
    static bool tried = false;
    static void *lib = NULL;

    if (tried)
        return lib != NULL;

    tried = true;

#define DLSYM(f) \
    f = get_library_symbol(lib, #f);            \
    assert(f != NULL)

    lib = open_library("libGL.so.1");
    if (lib == NULL)
        return false;

    DLSYM(glXGetProcAddress);

#define GPA(f) \
    f = glXGetProcAddress((const unsigned char*)#f);    \
    assert(f != NULL)

    GPA(glXChooseFBConfig);
    GPA(glXChooseVisual);
    GPA(glXCreateContext);
    GPA(glXDestroyContext);
    GPA(glXMakeCurrent);
    GPA(glXQueryExtension);
    GPA(glXQueryExtensionsString);
    GPA(glXSwapBuffers);

    return true;
}

static void read_extensions(Display *dpy);
static bool check_extension(const char *ext);
static bool GLX_EXT_create_context_es2_profile_supported = false;

static void
get_extension_procedure_address(Display *dpy)
{
    static bool done = false;
    if (done)
        return;
    else
        done = true;

    read_extensions(dpy);
    if (check_extension("GLX_ARB_create_context")) {
        GPA(glXCreateContextAttribsARB);
    }
    if (check_extension("GLX_EXT_create_context_es2_profile")) {
        GLX_EXT_create_context_es2_profile_supported = true;
    }
    if (check_extension("GLX_MESA_swap_control")) {
        GPA(glXGetSwapIntervalMESA);
        GPA(glXSwapIntervalMESA);
    }
    if (check_extension("GLX_EXT_swap_control")) {
        GPA(glXSwapIntervalEXT);
    }
}

static GLXContext ctx = None;
static bool set_glx_swap_vsync(Display *dpy, GLXDrawable draw, bool vsync);

static bool
create_glx_context_no_attrib(Display *dpy, GLXDrawable draw)
{
    if (drawer->api_type != API_OPENGL_COMPAT)
        return false;

    XVisualInfo *vis_info = NULL;
    ctx = glXCreateContext(dpy, vis_info, NULL, True);
    return (ctx != None);
}

static bool
create_glx_context_attrib(Display *dpy, GLXDrawable draw)
{
    const int fbattr[] = {
        GLX_X_RENDERABLE, True,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_DOUBLEBUFFER, True,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 16,
        GLX_STENCIL_SIZE, 0,
        GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB, True,
        None,
    };
    int num_cfg;
    GLXFBConfig *fbcfg =
        glXChooseFBConfig(dpy, DefaultScreen(dpy), fbattr, &num_cfg);
    if (fbcfg == NULL)
        return false;

    static const int compat_attr[] = {
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
        None,
    };
    static const int core_attr[] = {
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
        GLX_CONTEXT_MINOR_VERSION_ARB, 2,
        None,
    };
    static const int es2_attr[] = {
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_ES2_PROFILE_BIT_EXT,
        GLX_CONTEXT_MAJOR_VERSION_ARB, 2,
        GLX_CONTEXT_MINOR_VERSION_ARB, 0,
        None,
    };
    const int *attr = NULL;
    switch (drawer->api_type) {
    case API_OPENGL_COMPAT:
        attr = compat_attr;
        break;
    case API_OPENGL_CORE:
        attr = core_attr;
        break;
    case API_OPENGL_ES2:
        if (GLX_EXT_create_context_es2_profile_supported)
            attr = es2_attr;
        break;
    default:
        break;
    }
    if (!attr)
        return false;
    ctx = glXCreateContextAttribsARB(dpy, fbcfg[0], NULL, True, attr);
    return (ctx != None);
}

bool
create_glx_context(Display *dpy, GLXDrawable draw)
{
    get_extension_procedure_address(dpy);

    bool created;
    if (glXCreateContextAttribsARB)
        created = create_glx_context_attrib(dpy, draw);
    else
        created = create_glx_context_no_attrib(dpy, draw);

    if (!created)
        return false;

    Bool cres = glXMakeCurrent(dpy, draw, ctx);
    if (cres != True)
        return false;

    set_glx_swap_vsync(dpy, draw, gears_options.vsync);

    return true;
}

bool
glx_swap_buffers(Display *dpy, GLXDrawable draw)
{
    assert(dpy != NULL && draw != None);
    glXSwapBuffers(dpy, draw);
    return true;
}

static const char *exts = NULL;

static void
read_extensions(Display *dpy)
{
    if (exts == NULL) {
        exts = glXQueryExtensionsString(dpy, DefaultScreen(dpy));
    }
}

static bool
check_extension(const char *ext)
{
    if (exts == NULL || ext == NULL)
        return false;

    int len = strlen(ext);
    if (len == 0)
        return false;

    const char *pos = exts;
    while (1) {
        pos = strstr(pos, ext);
        if (pos == NULL)
            return false;
        if (pos[len] == ' ' || pos[len] == '\0')
            return true;
        pos = strchr(pos + len, ' ');
        if (pos == NULL)
            return false;
    }
}

static bool
set_glx_swap_vsync(Display *dpy, GLXDrawable draw, bool vsync)
{
    int interval = vsync ? 1 : 0;
    if (glXSwapIntervalMESA) {
        glXSwapIntervalMESA(interval);
        return true;
    } else if (glXSwapIntervalEXT) {
        glXSwapIntervalEXT(dpy, draw, interval);
        return true;
    } else {
        return false;
    }
}
