/* Jordan Justen : gears3d is public domain */

#include "glfn.h"
#include "gl.h"
#include "main.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define DECL_PGLFN(fn) PFN_##fn fn = NULL

DECL_PGLFN(glCreateShader);
DECL_PGLFN(glAttachShader);
DECL_PGLFN(glCompileShader);
DECL_PGLFN(glCreateProgram);
DECL_PGLFN(glDeleteProgram);
DECL_PGLFN(glDeleteShader);
DECL_PGLFN(glEnableVertexAttribArray);
DECL_PGLFN(glGetAttribLocation);
DECL_PGLFN(glGetProgramInfoLog);
DECL_PGLFN(glGetProgramiv);
DECL_PGLFN(glGetShaderInfoLog);
DECL_PGLFN(glGetShaderiv);
DECL_PGLFN(glGetUniformLocation);
DECL_PGLFN(glLinkProgram);
DECL_PGLFN(glShaderSource);
DECL_PGLFN(glUniform1f);
DECL_PGLFN(glUniform1i);
DECL_PGLFN(glUniform3fv);
DECL_PGLFN(glUniformMatrix4fv);
DECL_PGLFN(glVertexAttribPointer);

DECL_PGLFN(glUseProgram);
DECL_PGLFN(glBindFragDataLocation);
DECL_PGLFN(glGenVertexArrays);
DECL_PGLFN(glBindVertexArray);
DECL_PGLFN(glGenBuffers);
DECL_PGLFN(glBindBuffer);
DECL_PGLFN(glBufferData);
DECL_PGLFN(glUniform4fv);
DECL_PGLFN(glDrawArraysInstanced);

DECL_PGLFN(glViewport);
DECL_PGLFN(glClear);
DECL_PGLFN(glClearColor);
DECL_PGLFN(glCullFace);
DECL_PGLFN(glDepthFunc);
DECL_PGLFN(glEnable);
DECL_PGLFN(glFrontFace);
DECL_PGLFN(glGetError);

DECL_PGLFN(glGetString);
DECL_PGLFN(glDrawArrays);

DECL_PGLFN(glBegin);
DECL_PGLFN(glCallList);
DECL_PGLFN(glDeleteLists);
DECL_PGLFN(glEnd);
DECL_PGLFN(glEndList);
DECL_PGLFN(glFrustum);
DECL_PGLFN(glGenLists);
DECL_PGLFN(glLightfv);
DECL_PGLFN(glLoadIdentity);
DECL_PGLFN(glMaterialfv);
DECL_PGLFN(glMatrixMode);
DECL_PGLFN(glNewList);
DECL_PGLFN(glNormal3f);
DECL_PGLFN(glPopMatrix);
DECL_PGLFN(glPushMatrix);
DECL_PGLFN(glRotatef);
DECL_PGLFN(glShadeModel);
DECL_PGLFN(glTranslatef);
DECL_PGLFN(glVertex3f);

DECL_PGLFN(glReadPixels);

bool
gl_locate_procs(gl_proc_getter getter)
{
    static bool tried = false;
    static void *lib = NULL;

    if (tried)
        return lib != NULL;

    tried = true;

#define GPA(f)                                  \
    f = getter((const unsigned char*)#f);       \
    assert(f != NULL)

    GPA(glClear);
    GPA(glClearColor);
    GPA(glEnable);
    GPA(glViewport);

    GPA(glReadPixels);

    if (drawer->api_type != API_OPENGL_COMPAT) {
        GPA(glAttachShader);
        GPA(glBindBuffer);
        GPA(glBindFragDataLocation);
        GPA(glBindVertexArray);
        GPA(glBufferData);
        GPA(glCompileShader);
        GPA(glCreateProgram);
        GPA(glCreateShader);
        GPA(glCullFace);
        GPA(glDeleteProgram);
        GPA(glDeleteShader);
        GPA(glDepthFunc);
        GPA(glDrawArrays);
        GPA(glDrawArraysInstanced);
        GPA(glEnableVertexAttribArray);
        GPA(glFrontFace);
        GPA(glGenBuffers);
        GPA(glGenVertexArrays);
        GPA(glGetAttribLocation);
        GPA(glGetError);
        GPA(glGetProgramInfoLog);
        GPA(glGetProgramiv);
        GPA(glGetShaderInfoLog);
        GPA(glGetShaderiv);
        GPA(glGetString);
        GPA(glGetUniformLocation);
        GPA(glLinkProgram);
        GPA(glShaderSource);
        GPA(glUniform1f);
        GPA(glUniform1i);
        GPA(glUniform3fv);
        GPA(glUniform4fv);
        GPA(glUniformMatrix4fv);
        GPA(glUseProgram);
        GPA(glVertexAttribPointer);
    } else {
        GPA(glBegin);
        GPA(glCallList);
        GPA(glDeleteLists);
        GPA(glEnd);
        GPA(glEndList);
        GPA(glFrustum);
        GPA(glGenLists);
        GPA(glLightfv);
        GPA(glLoadIdentity);
        GPA(glMaterialfv);
        GPA(glMatrixMode);
        GPA(glNewList);
        GPA(glNormal3f);
        GPA(glPopMatrix);
        GPA(glPushMatrix);
        GPA(glRotatef);
        GPA(glShadeModel);
        GPA(glTranslatef);
        GPA(glVertex3f);
    }

    return true;
}
