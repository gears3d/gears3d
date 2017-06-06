/* Jordan Justen : gears3d is public domain */

#include "gl.h"

#include <epoxy/gl.h>

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static GLuint
compile_shader(GLenum target, const char *source)
{
    GLuint shader;
    shader = glCreateShader(target);
    if (shader == 0) {
        return shader;
    }

    glShaderSource(shader, 1, (const GLchar **) &source, NULL);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        GLint size;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &size);
        GLchar info[size];
        glGetShaderInfoLog(shader, size, NULL, info);
        printf("Shader compilation failed:\n%s\n", (char*)info);
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

static GLuint
build_program(GLuint *shaders, int num_shaders)
{
    int i;

    if (shaders == NULL || num_shaders <= 0)
        return 0;

    for(i = 0; i < num_shaders; i++)
        if (shaders[i] == 0)
            return 0;

    GLuint program = glCreateProgram();
    if (program == 0)
        return 0;

    for(i = 0; i < num_shaders; i++)
        glAttachShader(program, shaders[i]);

    glLinkProgram(program);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status) {
        GLint size;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &size);
        GLchar info[size];
        glGetProgramInfoLog(program, size, NULL, info);
        printf("Shader program link failed:\n%s\n", info);
        glDeleteProgram(program);
        program = 0;
        return 0;
    }

    return program;
}

static GLuint
build_program_from_sources(GLenum target, const char *source, ...)
{
    va_list va;
    int num_sources, num_compiled;
    GLenum va_target;
    const char *va_source;
    GLuint program = 0;

    va_start(va, source);
    for (num_sources = 1; ; num_sources++) {
        va_target = va_arg(va, GLenum);
        if (va_target == 0)
            break;
        va_arg(va, const char *);
    }
    va_end(va);

    GLuint *shaders = calloc(num_sources, sizeof(GLuint *));
    if (shaders == NULL)
        return 0;

    va_start(va, source);
    for (num_compiled = 0; num_compiled < num_sources; num_compiled++) {
        if (num_compiled == 0) {
            va_target = target;
            va_source = source;
        } else {
            va_target = va_arg(va, GLenum);
            va_source = va_arg(va, const char *);
        }
        assert(va_target != 0 && va_source != NULL);

        shaders[num_compiled] = compile_shader(va_target, va_source);
        if(shaders[num_compiled] == 0)
            break;
    }
    va_end(va);

    if (num_compiled == num_sources)
        program = build_program(shaders, num_compiled);

    for (num_compiled--; num_compiled >= 0; num_compiled--) {
        assert(shaders[num_compiled] != 0);
        glDeleteShader(shaders[num_compiled]);
    }
    free(shaders);
    return program;
}

GLuint
gl_program_vf_str(const char *vs, const char *fs)
{
    return build_program_from_sources(GL_VERTEX_SHADER, vs,
                                      GL_FRAGMENT_SHADER, fs,
                                      0);
}

GLuint
gl_program_vgf_str(const char *vs, const char *gs, const char *fs)
{
    return build_program_from_sources(GL_VERTEX_SHADER, vs,
                                      GL_GEOMETRY_SHADER, gs,
                                      GL_FRAGMENT_SHADER, fs,
                                      0);
}

static GLint
attrib_location(GLuint program, const char *name)
{
    return glGetAttribLocation(program, name);
}

bool
gl_attrib_ptr(GLuint program, const char *name, GLint size, GLenum type,
              GLboolean normalized, GLsizei stride,
              GLuint offset)
{
    GLint location = attrib_location(program, name);
    if (location < 0) {
        return false;
    }

    glVertexAttribPointer(location, size, type, normalized, stride,
                          (GLvoid*)(uintptr_t) offset);
    glEnableVertexAttribArray(location);

    return true;
}

bool
gl_uniform1f(GLuint program, const char *name, GLfloat value)
{
    GLint location = glGetUniformLocation(program, name);
    if (location < 0) {
        return false;
    }

    glUniform1f(location, value);
    return true;
}

bool
gl_uniform1i(GLuint program, const char *name, GLuint value)
{
    GLint location = glGetUniformLocation(program, name);
    if (location < 0) {
        return false;
    }

    glUniform1i(location, value);
    return true;
}

bool
gl_uniform3fv(GLuint program, const char *name, const GLfloat* value)
{
    GLint location = glGetUniformLocation(program, name);
    if (location < 0) {
        return false;
    }

    glUniform3fv(location, 1, value);
    return true;
}

bool
gl_uniform_mat4(GLuint program, const char *name, const float* mat4)
{
    GLint location = glGetUniformLocation(program, name);
    if (location < 0) {
        return false;
    }

    glUniformMatrix4fv(location, 1, GL_FALSE, mat4);
    return true;
}
