/* Jordan Justen : gears3d is public domain */

#include "egl.h"
#include "gl.h"
#include "load_lib.h"
#include "main.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define DECL_PEGLFN(fn) PFN_##fn fn = NULL

DECL_PEGLFN(eglBindAPI);
DECL_PEGLFN(eglChooseConfig);
DECL_PEGLFN(eglCreateContext);
DECL_PEGLFN(eglCreateWindowSurface);
DECL_PEGLFN(eglGetConfigs);
DECL_PEGLFN(eglGetDisplay);
DECL_PEGLFN(eglGetError);
DECL_PEGLFN(eglGetProcAddress);
DECL_PEGLFN(eglInitialize);
DECL_PEGLFN(eglMakeCurrent);
DECL_PEGLFN(eglQueryContext);
DECL_PEGLFN(eglQueryString);
DECL_PEGLFN(eglQuerySurface);
DECL_PEGLFN(eglSwapBuffers);
DECL_PEGLFN(eglSwapInterval);

static const char *
error_to_str(EGLint error)
{
    #define EGLERRSTR(e) e: return #e
    switch(error) {
    case EGLERRSTR(EGL_SUCCESS);
    case EGLERRSTR(EGL_NOT_INITIALIZED);
    case EGLERRSTR(EGL_BAD_ACCESS);
    case EGLERRSTR(EGL_BAD_ALLOC);
    case EGLERRSTR(EGL_BAD_ATTRIBUTE);
    case EGLERRSTR(EGL_BAD_CONTEXT);
    case EGLERRSTR(EGL_BAD_CONFIG);
    case EGLERRSTR(EGL_BAD_CURRENT_SURFACE);
    case EGLERRSTR(EGL_BAD_DISPLAY);
    case EGLERRSTR(EGL_BAD_SURFACE);
    case EGLERRSTR(EGL_BAD_MATCH);
    case EGLERRSTR(EGL_BAD_PARAMETER);
    case EGLERRSTR(EGL_BAD_NATIVE_PIXMAP);
    case EGLERRSTR(EGL_BAD_NATIVE_WINDOW);
    case EGLERRSTR(EGL_CONTEXT_LOST);
    default: return NULL;
    }
    #undef EGLERRSTR
}

static void
show_egl_error(void)
{
    EGLint error = eglGetError();
    const char *s = error_to_str(error);
    if (s)
        fprintf(stderr, "EGL error: %s\n", s);
    else
        fprintf(stderr, "EGL error: 0x%x\n", error);
}

bool
load_egl_library(void)
{
    static bool tried = false;
    static void *lib = NULL;

    if (tried)
        return lib != NULL;

    tried = true;

    #define DLSYM(f)                            \
        f = get_library_symbol(lib, #f);        \
        assert(f != NULL)

    lib = open_library("libEGL.so.1");
    if (lib == NULL)
        return false;

    DLSYM(eglGetProcAddress);

    #undef DLSYM

    #define GPA(f)                              \
        f = eglGetProcAddress((const char*)#f); \
        assert(f != NULL)

    GPA(eglBindAPI);
    GPA(eglChooseConfig);
    GPA(eglCreateContext);
    GPA(eglCreateWindowSurface);
    GPA(eglGetConfigs);
    GPA(eglGetDisplay);
    GPA(eglGetError);
    GPA(eglInitialize);
    GPA(eglMakeCurrent);
    GPA(eglQueryContext);
    GPA(eglQueryString);
    GPA(eglQuerySurface);
    GPA(eglSwapBuffers);
    GPA(eglSwapInterval);

    #undef GPA

    gl_locate_procs((gl_proc_getter)eglGetProcAddress);

    return true;
}

static void read_extensions(EGLDisplay *dpy);
static bool check_extension(const char *ext);
#define EGL_VER_U32(major, minor) ((major << 16) | minor)
static uint32_t egl_version = EGL_VER_U32(0, 0);
static bool EGL_KHR_create_context_supported = false;
static EGLDisplay dpy = NULL;
static EGLSurface surf = NULL;
static EGLConfig config;

static void
get_extension_procedure_address(void)
{
    static bool done = false;
    if (done)
        return;
    else
        done = true;

    read_extensions(dpy);
    if (check_extension("EGL_KHR_create_context")) {
        EGL_KHR_create_context_supported = true;
    }
}

static EGLContext ctx = EGL_NO_CONTEXT;
static bool set_egl_swap_vsync(EGLDisplay *dpy, bool vsync);

static bool
create_egl_context_attrib(void)
{
    EGLint major, minor;
    if (!eglInitialize(dpy, &major, &minor))
        return false;

    egl_version = EGL_VER_U32(major, minor);

    if (drawer->api_type != API_OPENGL_ES2) {
        if(egl_version < EGL_VER_U32(1, 4))
            return false;
        eglBindAPI(EGL_OPENGL_API);
    }

    get_extension_procedure_address();

    const EGLint attr_list[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 16,
        EGL_STENCIL_SIZE, 0,
        EGL_NONE,
    };
    int num_cfg;
    if (!eglGetConfigs(dpy, NULL, 0, &num_cfg))
        return false;

    EGLConfig *configs = calloc(num_cfg, sizeof(EGLConfig));
    if (!eglChooseConfig(dpy, attr_list, configs, num_cfg, &num_cfg))
        return false;

    config = configs[0];
    free(configs);

    static const int compat_attr[] = {
        EGL_CONTEXT_MAJOR_VERSION, 1,
        EGL_CONTEXT_MINOR_VERSION, 2,
        EGL_NONE,
    };
    static const int core_attr[] = {
        EGL_CONTEXT_OPENGL_PROFILE_MASK,
            EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 2,
        EGL_NONE,
    };
    static const int es2_attr[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE,
    };
    const int *attr = NULL;
    switch (drawer->api_type) {
    case API_OPENGL_COMPAT:
        if (EGL_KHR_create_context_supported)
            attr = compat_attr;
        break;
    case API_OPENGL_CORE:
        if (EGL_KHR_create_context_supported)
            attr = core_attr;
        break;
    case API_OPENGL_ES2:
        attr = es2_attr;
        break;
    default:
        break;
    }
    if (!attr)
        return false;

    ctx = eglCreateContext(dpy, config, EGL_NO_CONTEXT, attr);
    if (ctx == EGL_NO_CONTEXT)
        show_egl_error();
    return (ctx != EGL_NO_CONTEXT);
}

bool
create_egl_context(EGLNativeDisplayType ndpy, EGLNativeWindowType nwnd)
{
    dpy = eglGetDisplay(ndpy);

    bool created;
    created = create_egl_context_attrib();

    if (!created)
        return false;

    surf = eglCreateWindowSurface(dpy, config, nwnd, NULL);

    EGLBoolean cres = eglMakeCurrent(dpy, surf, surf, ctx);
    if (cres != EGL_TRUE)
        return false;

    set_egl_swap_vsync(dpy, gears_options.vsync);

    return true;
}

bool
egl_swap_buffers()
{
    assert(dpy != NULL && surf != EGL_NO_CONTEXT);
    eglSwapBuffers(dpy, surf);
    gl_post_swap_buffer();

    return true;
}

static const char *exts = NULL;

static void
read_extensions(EGLDisplay *dpy)
{
    if (exts == NULL) {
        exts = eglQueryString(dpy, EGL_EXTENSIONS);
        if (exts == NULL)
            show_egl_error();
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
set_egl_swap_vsync(EGLDisplay *dpy, bool vsync)
{
    int interval = vsync ? 1 : 0;
    eglSwapInterval(dpy, interval);
    return true;
}
