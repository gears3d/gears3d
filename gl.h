/* Jordan Justen : gears3d is public domain */

#ifndef __gl_h_included__
#define __gl_h_included__

#include <stdbool.h>

typedef void* (*gl_proc_getter)(const unsigned char *);

bool gl_locate_procs(gl_proc_getter getter);

GLuint gl_program_vf_str(const char *vs, const char *fs);
GLuint gl_program_vgf_str(const char *vs, const char *gs, const char *fs);

bool gl_uniform1f(GLuint program, const char *name, GLfloat value);
bool gl_uniform1i(GLuint program, const char *name, GLuint value);
bool gl_uniform3fv(GLuint program, const char *name, const GLfloat* value);
bool gl_uniform_mat4(GLuint program, const char *name, const GLfloat* mat4);

bool gl_attrib_ptr(GLuint program, const char *name, GLint size, GLenum type,
                   GLboolean normalized, GLsizei stride, GLuint offset);

bool gl_post_swap_buffer(void);

#endif
