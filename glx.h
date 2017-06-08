/* Jordan Justen : gears3d is public domain */

#ifndef __glx_h_included__
#define __glx_h_included__

#include <stdbool.h>
#include <GL/glx.h>

bool load_glx_library(void);
bool create_glx_context(Display *dpy, GLXDrawable draw);
bool glx_swap_buffers(Display *dpy, GLXDrawable draw);

#define GLXFN(fn) p_##fn

#define glXChooseFBConfig GLXFN(glXChooseFBConfig)
#define glXChooseVisual GLXFN(glXChooseVisual)
#define glXCreateContext GLXFN(glXCreateContext)
#define glXCreateContextAttribsARB GLXFN(glXCreateContextAttribsARB)
#define glXDestroyContext GLXFN(glXDestroyContext)
#define glXGetProcAddress GLXFN(glXGetProcAddress)
#define glXGetSwapIntervalMESA GLXFN(glXGetSwapIntervalMESA)
#define glXMakeCurrent GLXFN(glXMakeCurrent)
#define glXQueryExtension GLXFN(glXQueryExtension)
#define glXQueryExtensionsString GLXFN(glXQueryExtensionsString)
#define glXSwapBuffers GLXFN(glXSwapBuffers)
#define glXSwapIntervalEXT GLXFN(glXSwapIntervalEXT)
#define glXSwapIntervalMESA GLXFN(glXSwapIntervalMESA)

#define DEF_PGLXFN(fn, ret, args)       \
    typedef ret (*PFN_##fn)args;        \
    extern PFN_##fn fn;

DEF_PGLXFN(glXChooseFBConfig, GLXFBConfig*, (Display*, int, const int*, int*))
DEF_PGLXFN(glXChooseVisual, XVisualInfo*, (Display*, int, int*))
DEF_PGLXFN(glXCreateContext, GLXContext, (Display*, XVisualInfo*, GLXContext,
                                          Bool))
DEF_PGLXFN(glXCreateContextAttribsARB, GLXContext, (Display*, GLXFBConfig,
                                                    GLXContext, Bool,
                                                    const int*))
DEF_PGLXFN(glXDestroyContext, void, (Display*, GLXContext))
DEF_PGLXFN(glXGetProcAddress, void*, (const unsigned char*))
DEF_PGLXFN(glXGetSwapIntervalMESA, int, (void))
DEF_PGLXFN(glXMakeCurrent, Bool, (Display*, GLXDrawable, GLXContext))
DEF_PGLXFN(glXQueryExtension, Bool, (Display*, int*, int*))
DEF_PGLXFN(glXQueryExtensionsString, const char*, (Display*, int))
DEF_PGLXFN(glXSwapBuffers, void, (Display*, GLXDrawable))
DEF_PGLXFN(glXSwapIntervalEXT, void, (Display*, GLXDrawable, int))
DEF_PGLXFN(glXSwapIntervalMESA, int, (unsigned int))

#endif
