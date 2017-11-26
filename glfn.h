/* Jordan Justen : gears3d is public domain */

#ifndef __glfn_h_included__
#define __glfn_h_included__

#include <stdbool.h>
#include <GL/glcorearb.h>
#include <GL/gl.h>

#define GLFN(fn) p_##fn

/* Used by all GL drawers */
#define glClear GLFN(glClear)
#define glClearColor GLFN(glClearColor)
#define glEnable GLFN(glEnable)
#define glReadPixels GLFN(glReadPixels)
#define glViewport GLFN(glViewport)

/* Used by Core profile and GLES drawers */
#define glAttachShader GLFN(glAttachShader)
#define glBindBuffer GLFN(glBindBuffer)
#define glBindFragDataLocation GLFN(glBindFragDataLocation)
#define glBindVertexArray GLFN(glBindVertexArray)
#define glBufferData GLFN(glBufferData)
#define glCompileShader GLFN(glCompileShader)
#define glCreateProgram GLFN(glCreateProgram)
#define glCreateShader GLFN(glCreateShader)
#define glCullFace GLFN(glCullFace)
#define glDeleteProgram GLFN(glDeleteProgram)
#define glDeleteShader GLFN(glDeleteShader)
#define glDepthFunc GLFN(glDepthFunc)
#define glDrawArrays GLFN(glDrawArrays)
#define glDrawArraysInstanced GLFN(glDrawArraysInstanced)
#define glDrawElements GLFN(glDrawElements)
#define glEnableVertexAttribArray GLFN(glEnableVertexAttribArray)
#define glFrontFace GLFN(glFrontFace)
#define glGenBuffers GLFN(glGenBuffers)
#define glGenVertexArrays GLFN(glGenVertexArrays)
#define glGetAttribLocation GLFN(glGetAttribLocation)
#define glGetError GLFN(glGetError)
#define glGetProgramInfoLog GLFN(glGetProgramInfoLog)
#define glGetProgramiv GLFN(glGetProgramiv)
#define glGetShaderInfoLog GLFN(glGetShaderInfoLog)
#define glGetShaderiv GLFN(glGetShaderiv)
#define glGetString GLFN(glGetString)
#define glGetUniformLocation GLFN(glGetUniformLocation)
#define glLinkProgram GLFN(glLinkProgram)
#define glShaderSource GLFN(glShaderSource)
#define glUniform1f GLFN(glUniform1f)
#define glUniform1i GLFN(glUniform1i)
#define glUniform3fv GLFN(glUniform3fv)
#define glUniform4fv GLFN(glUniform4fv)
#define glUniformMatrix4fv GLFN(glUniformMatrix4fv)
#define glUseProgram GLFN(glUseProgram)
#define glVertexAttribPointer GLFN(glVertexAttribPointer)

/* Used by Compatibility profile drawer */
#define glBegin GLFN(glBegin)
#define glCallList GLFN(glCallList)
#define glDeleteLists GLFN(glDeleteLists)
#define glEnd GLFN(glEnd)
#define glEndList GLFN(glEndList)
#define glFrustum GLFN(glFrustum)
#define glGenLists GLFN(glGenLists)
#define glLightfv GLFN(glLightfv)
#define glLoadIdentity GLFN(glLoadIdentity)
#define glMaterialfv GLFN(glMaterialfv)
#define glMatrixMode GLFN(glMatrixMode)
#define glNewList GLFN(glNewList)
#define glNormal3f GLFN(glNormal3f)
#define glPopMatrix GLFN(glPopMatrix)
#define glPushMatrix GLFN(glPushMatrix)
#define glRotatef GLFN(glRotatef)
#define glShadeModel GLFN(glShadeModel)
#define glTranslatef GLFN(glTranslatef)
#define glVertex3f GLFN(glVertex3f)

#define DEF_PGLFN(fn, ret, args)                \
    typedef ret (APIENTRY *PFN_##fn)args;     \
    extern PFN_##fn fn;

/* Used by all GL drawers */
DEF_PGLFN(glClear, void, (GLbitfield))
DEF_PGLFN(glClearColor, void, (GLfloat, GLfloat, GLfloat, GLfloat))
DEF_PGLFN(glEnable, void, (GLenum))
DEF_PGLFN(glReadPixels, void, (GLint, GLint, GLsizei, GLsizei, GLenum, GLenum,
                               GLvoid*))
DEF_PGLFN(glViewport, void, (GLint, GLint, GLsizei, GLsizei))

/* Used by Core profile and GLES drawers */
DEF_PGLFN(glAttachShader, void, (GLuint, GLuint))
DEF_PGLFN(glBindBuffer, void, (GLenum, GLuint))
DEF_PGLFN(glBindFragDataLocation, void, (GLuint, GLuint, const GLchar*))
DEF_PGLFN(glBindVertexArray, void, (GLuint))
DEF_PGLFN(glBufferData, void, (GLenum, GLsizeiptr, const void*, GLenum))
DEF_PGLFN(glCompileShader, void, (GLuint))
DEF_PGLFN(glCreateProgram, GLuint, (void))
DEF_PGLFN(glCreateShader, GLuint, (GLenum))
DEF_PGLFN(glCullFace, void, (GLenum))
DEF_PGLFN(glDeleteProgram, void, (GLuint))
DEF_PGLFN(glDeleteShader, void, (GLuint))
DEF_PGLFN(glDepthFunc, void, (GLenum))
DEF_PGLFN(glDrawArrays, void, (GLenum, GLint, GLsizei))
DEF_PGLFN(glDrawArraysInstanced, void, (GLenum, GLint, GLsizei, GLsizei))
DEF_PGLFN(glDrawElements, void, (GLenum, GLsizei, GLenum, const void*))
DEF_PGLFN(glEnableVertexAttribArray, void, (GLuint))
DEF_PGLFN(glFrontFace, void, (GLenum))
DEF_PGLFN(glGenBuffers, void, (GLsizei, GLuint*))
DEF_PGLFN(glGenVertexArrays, void, (GLsizei, GLuint*))
DEF_PGLFN(glGetAttribLocation, GLint, (GLuint, const GLchar*))
DEF_PGLFN(glGetError, GLenum, (void))
DEF_PGLFN(glGetProgramInfoLog, void, (GLuint, GLsizei, GLsizei*, GLchar*))
DEF_PGLFN(glGetProgramiv, void, (GLuint, GLenum, GLint*))
DEF_PGLFN(glGetShaderInfoLog, void, (GLuint, GLsizei, GLsizei*, GLchar*))
DEF_PGLFN(glGetShaderiv, void, (GLuint, GLenum, GLint*))
DEF_PGLFN(glGetString, const GLubyte*, (GLenum))
DEF_PGLFN(glGetUniformLocation, GLint, (GLuint, const GLchar*))
DEF_PGLFN(glLinkProgram, void, (GLuint))
DEF_PGLFN(glShaderSource, void, (GLuint, GLsizei, const GLchar *const*,
                                 const GLint*))
DEF_PGLFN(glUniform1f, void, (GLint, GLfloat))
DEF_PGLFN(glUniform1i, void, (GLint, GLint))
DEF_PGLFN(glUniform3fv, void, (GLint, GLsizei, const GLfloat*))
DEF_PGLFN(glUniform4fv, void, (GLint, GLsizei, const GLfloat*))
DEF_PGLFN(glUniformMatrix4fv, void, (GLint, GLsizei, GLboolean,
                                     const GLfloat*))
DEF_PGLFN(glUseProgram, void, (GLuint))
DEF_PGLFN(glVertexAttribPointer, void, (GLuint, GLint, GLenum, GLboolean,
                                        GLsizei, const void*))

/* Used by Compatibility profile drawer */
DEF_PGLFN(glBegin, void, (GLenum))
DEF_PGLFN(glCallList, void, (GLuint))
DEF_PGLFN(glDeleteLists, void, (GLuint, GLsizei))
DEF_PGLFN(glEnd, void, (void))
DEF_PGLFN(glEndList, void, (void))
DEF_PGLFN(glFrustum, void, (GLdouble, GLdouble, GLdouble, GLdouble, GLdouble,
                            GLdouble))
DEF_PGLFN(glGenLists, GLuint, (GLsizei))
DEF_PGLFN(glLightfv, void, (GLenum, GLenum, const GLfloat*))
DEF_PGLFN(glLoadIdentity, void, (void))
DEF_PGLFN(glMaterialfv, void, (GLenum, GLenum, const GLfloat*))
DEF_PGLFN(glMatrixMode, void, (GLenum))
DEF_PGLFN(glNewList, void, (GLuint, GLenum))
DEF_PGLFN(glNormal3f, void, (GLfloat, GLfloat, GLfloat))
DEF_PGLFN(glPopMatrix, void, (void))
DEF_PGLFN(glPushMatrix, void, (void))
DEF_PGLFN(glRotatef, void, (GLfloat, GLfloat, GLfloat, GLfloat))
DEF_PGLFN(glShadeModel, void, (GLenum))
DEF_PGLFN(glTranslatef, void, (GLfloat, GLfloat, GLfloat))
DEF_PGLFN(glVertex3f, void, (GLfloat, GLfloat, GLfloat))

#undef DEF_PGLFN
#define DEF_PGLFN(fn, pty)                      \
    typedef pty PFN_##fn;                       \
    extern PFN_##fn fn;

#endif
