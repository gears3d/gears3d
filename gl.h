/* Gears. Draw them. In 3D.
 *
 * Jordan Justen : gears3d is public domain
 */

#ifndef __gl_h_included__
#define __gl_h_included__

#include <epoxy/gl.h>
#include <stdbool.h>

GLuint gl_program_vf_str(const char *vs, const char *fs);
GLuint gl_program_vgf_str(const char *vs, const char *gs, const char *fs);

bool gl_uniform1f(GLuint program, const char *name, GLfloat value);
bool gl_uniform1i(GLuint program, const char *name, GLuint value);
bool gl_uniform3fv(GLuint program, const char *name, const GLfloat* value);
bool gl_uniform_mat4(GLuint program, const char *name, const GLfloat* mat4);

bool gl_attrib_ptr(GLuint program, const char *name, GLint size, GLenum type,
                   GLboolean normalized, GLsizei stride, GLuint offset);

#endif
