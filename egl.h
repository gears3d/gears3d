/* Jordan Justen : gears3d is public domain */

#ifndef __egl_h_included__
#define __egl_h_included__

#include <stdbool.h>
#include <stdint.h>

#include <EGL/egl.h>
#include <GL/gl.h>

bool load_egl_library(void);
bool create_egl_context(EGLNativeDisplayType ndpy, EGLNativeWindowType nwnd);
bool egl_swap_buffers();

#define EGLFN(fn) p_##fn

#define eglBindAPI EGLFN(eglBindAPI)
#define eglChooseConfig EGLFN(eglChooseConfig)
#define eglCreateContext EGLFN(eglCreateContext)
#define eglCreateWindowSurface EGLFN(eglCreateWindowSurface)
#define eglGetDisplay EGLFN(eglGetDisplay)
#define eglGetConfigs EGLFN(eglGetConfigs)
#define eglGetError EGLFN(eglGetError)
#define eglGetProcAddress EGLFN(eglGetProcAddress)
#define eglInitialize EGLFN(eglInitialize)
#define eglMakeCurrent EGLFN(eglMakeCurrent)
#define eglQueryContext EGLFN(eglQueryContext)
#define eglQueryString EGLFN(eglQueryString)
#define eglQuerySurface EGLFN(eglQuerySurface)
#define eglSwapBuffers EGLFN(eglSwapBuffers)
#define eglSwapInterval EGLFN(eglSwapInterval)

#define DEF_PEGLFN(fn, ret, args)               \
    typedef ret (EGLAPIENTRY *PFN_##fn)args;    \
    extern PFN_##fn fn;

DEF_PEGLFN(eglBindAPI, EGLBoolean, (EGLenum))
DEF_PEGLFN(eglChooseConfig, EGLBoolean, (EGLDisplay, const EGLint*, EGLConfig*,
                                         EGLint, EGLint*))
DEF_PEGLFN(eglCreateContext, EGLContext, (EGLDisplay, EGLConfig, EGLContext,
                                          const EGLint *))
DEF_PEGLFN(eglCreateWindowSurface, EGLSurface, (EGLDisplay, EGLConfig,
                                                EGLNativeWindowType,
                                                const EGLint *))
DEF_PEGLFN(eglGetConfigs, EGLBoolean, (EGLDisplay, EGLConfig*, EGLint, EGLint*))
DEF_PEGLFN(eglGetDisplay, EGLDisplay, (EGLNativeDisplayType))
DEF_PEGLFN(eglGetError, EGLint, (void))
DEF_PEGLFN(eglGetProcAddress, const void*, (const char *))
DEF_PEGLFN(eglInitialize, EGLBoolean, (EGLDisplay, EGLint*, EGLint*))
DEF_PEGLFN(eglMakeCurrent, EGLBoolean, (EGLDisplay, EGLSurface, EGLSurface,
                                        EGLContext))
DEF_PEGLFN(eglQueryContext, EGLBoolean, (EGLDisplay, EGLContext, EGLint,
                                         EGLint*))
DEF_PEGLFN(eglQueryString, const char*, (EGLDisplay, EGLint))
DEF_PEGLFN(eglQuerySurface, EGLBoolean, (EGLDisplay, EGLSurface, EGLint,
                                         EGLint*))
DEF_PEGLFN(eglSwapBuffers, EGLBoolean, (EGLDisplay, EGLSurface))
DEF_PEGLFN(eglSwapInterval, EGLBoolean, (EGLDisplay, EGLint))

#endif
