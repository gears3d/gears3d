/* Jordan Justen : gears3d is public domain */

#include "main.h"
#include "gl.h"
#include "g_math.h"
#include "vert_buf.h"
#include "sdl_fw.h"

static GLuint program = 0;
static GLuint vao = 0;
static GLuint vert_bo = 0;

#pragma pack(1)

struct uniform_loc {
    GLint loc;
    const char *name;
};

static struct {
    struct uniform_loc model;
    struct uniform_loc view;
    struct uniform_loc projection;
    struct uniform_loc gear_angle;
    struct uniform_loc tooth_angle;
    struct uniform_loc gear_color;
} uniforms = {
#define UNIFORM(u) .u = { -1, #u }
    UNIFORM(model),
    UNIFORM(view),
    UNIFORM(projection),
    UNIFORM(gear_angle),
    UNIFORM(tooth_angle),
    UNIFORM(gear_color),
};

static bool
lookup_uniform_locations(void)
{
    int i;
    for (i = 0; i < sizeof(uniforms) / sizeof(struct uniform_loc); i++) {
        struct uniform_loc *u = &((struct uniform_loc *)&uniforms)[i];
        assert(u->loc == -1 && u->name != NULL);
        u->loc = glGetUniformLocation(program, u->name);
        if (u->loc < 0)
            return false;
    }

    return true;
}

#pragma pack()

static void
set_gl_core_attributes()
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
}

#define GEARS 3

struct gear_info {
    int teeth;
    float inner_radius, outer_radius, width, tooth_depth;
    float angle_rate;
    float angle_adjust;
    struct vec3 translate;
    GLfloat color[4];
    mat4 model;
    float angle;
    uint32_t num_vertices, vertex_buf_offset;
};

static struct gear_info gears[GEARS] = {
    {
        .teeth = 20, .inner_radius = 1.0, .outer_radius = 4.0, .width = 1.0,
        .tooth_depth = 0.7,
        .angle_rate = 1.0, .angle_adjust = 0.0,
        .translate = { -3.0, -2.0 },
        .color = { 0.8, 0.1, 0.0, 1.0 },
    },
    {
        .teeth = 10, .inner_radius = 0.5, .outer_radius = 2.0, .width = 2.0,
        .tooth_depth = 0.7,
        .angle_rate = -2.0, .angle_adjust = (M_PI * -9.0 / 180.0),
        .translate = { 3.1, -2.0 },
        .color = { 0.0, 0.8, 0.2, 1.0 },
    },
    {
        .teeth = 10, .inner_radius = 1.3, .outer_radius = 2.0, .width = 0.5,
        .tooth_depth = 0.7,
        .angle_rate = -2.0, .angle_adjust = (M_PI * -25.0 / 180.0),
        .translate = { -3.1, 4.2 },
        .color = { 0.2, 0.2, 1.0, 1.0 },
    },
};

static void
rotate_gears(float x, float y, float z)
{
    float m4[16];
    float tmp[16];
    int i;

    for (i = 0; i < GEARS; i++) {
        rotate(m4, x, 1.0, 0.0, 0.0);
        if (y != 0.0) {
            rotate(tmp, y, 0.0, 1.0, 0.0);
            mult_m4m4(m4, m4, tmp);
        }
        if (z != 0.0) {
            rotate(tmp, z, 0.0, 0.0, 1.0);
            mult_m4m4(m4, m4, tmp);
        }
        translate(tmp, gears[i].translate.x, gears[i].translate.y, 0.0);
        mult_m4m4(m4, m4, tmp);
        memcpy(&gears[i].model, m4, sizeof(m4));
    }
}

static void
reshape(int width, int height)
{
    GLfloat h = (GLfloat) height / (GLfloat) width;
    float m4[16];

    glViewport(0, 0, (GLint) width, (GLint) height);

    translate(m4, 0.0, 0.0, -40.0);
    glUniformMatrix4fv(uniforms.view.loc, 1, GL_FALSE, m4);

    frustum(m4, -1.0, 1.0, -h, h, 5.0, 200.0);
    glUniformMatrix4fv(uniforms.projection.loc, 1, GL_FALSE, m4);
}

static void
win_resize(int width, int height)
{
    reshape(width, height);
}

static void
set_gl_core_state()
{
    int i;
    bool ok;

    glClearColor(0.0, 0.0, 0.0, 1.0);

#define L(s) s "\n"
    static const char *vs_src =
        L("#version 150")
        L("")
        L("uniform mat4 model;")
        L("uniform mat4 view;")
        L("uniform mat4 projection;")
        L("")
        L("uniform float gear_angle;")
        L("uniform float tooth_angle;")
        L("")
        L("in vec3 vertex;")
        L("in vec3 rel_norm;")
        L("out vec3 norm;")
        L("out vec3 light_dir;")
        L("")
        L("const vec3 light_pos = vec3(5.0, 5.0, 10.0);")
        L("")
        L("void main()")
        L("{")
        L("    float ang = tooth_angle * gl_InstanceID + gear_angle;")
        L("    mat2 rotz = mat2(vec2(cos(ang), sin(ang)),")
        L("                     vec2(-sin(ang), cos(ang)));")
        L("")
        L("    vec3 pos = vec3(rotz * vertex.xy, vertex.z);")
        L("    vec4 m_pos = model * vec4(pos, 1.0);")
        L("    m_pos = vec4(m_pos.xyz / m_pos.w, 1.0);")
        L("    gl_Position = projection * view * m_pos;")
        L("")
        L("    light_dir = normalize(light_pos - m_pos.xyz);")
        L("")
        L("    vec3 n_pos = vertex + rel_norm;")
        L("    n_pos = vec3(rotz * n_pos.xy, n_pos.z);")
        L("    vec4 m_norm = model * vec4(n_pos, 1.0);")
        L("    norm = normalize((m_norm.xyz / m_norm.w) - m_pos.xyz);")
        L("}");

    static const char *fs_src =
        L("#version 150")
        L("")
        L("in vec3 norm;")
        L("in vec3 light_dir;")
        L("out vec4 color;")
        L("")
        L("uniform vec4 gear_color;")
        L("")
        L("void main()")
        L("{")
        L("    float light_ref = clamp(0.0+dot(norm, light_dir), -0.0, 1.0);")
        L("    float light = clamp(0.2+light_ref, 0.1, 1.0);")
        L("    color = vec4(light * gear_color.xyz, 1.0);")
        L("}");

    assert(glGetError() == GL_NO_ERROR);
    program = gl_program_vf_str(vs_src, fs_src);
    assert(program);
    glUseProgram(program);
    glBindFragDataLocation(program, 0, "color");

    ok = lookup_uniform_locations();
    assert(ok);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    uint32_t total_vertex_count = 0;
    for (i = 0; i < GEARS; i++) {
        gears[i].vertex_buf_offset = total_vertex_count;
        gears[i].num_vertices = tooth_vertex_count();
        total_vertex_count += gears[i].num_vertices;
    }

    struct gear_vert *vert_buffer =
        malloc(total_vertex_count * sizeof(struct gear_vert));
    assert(vert_buffer != NULL);

    for (i = 0; i < GEARS; i++) {
        fill_tooth_vertices(vert_buffer + gears[i].vertex_buf_offset,
                            gears[i].inner_radius, gears[i].outer_radius,
                            gears[i].width, gears[i].teeth,
                            gears[i].tooth_depth);
    }

    glGenBuffers(1, &vert_bo);
    glBindBuffer(GL_ARRAY_BUFFER, vert_bo);
    glBufferData(GL_ARRAY_BUFFER, total_vertex_count * sizeof(struct gear_vert),
                 vert_buffer, GL_STATIC_DRAW);
    ok = gl_attrib_ptr(program, "vertex",
                       sizeof(vert_buffer->pos) / sizeof(GLfloat),
                       GL_FLOAT, GL_FALSE, sizeof(*vert_buffer), 0);
    assert(ok);
    ok = gl_attrib_ptr(program, "rel_norm",
                       sizeof(vert_buffer->norm) / sizeof(GLfloat),
                       GL_FLOAT, GL_FALSE, sizeof(*vert_buffer),
                       (uint8_t*)&vert_buffer->norm - (uint8_t*)vert_buffer);
    assert(ok);
    free(vert_buffer);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glBindBuffer(GL_ARRAY_BUFFER, vert_bo);
    assert(glGetError() == GL_NO_ERROR);

    rotate_gears(20.0 / 180.0 * M_PI, 30.0 / 180.0 * M_PI, 0.0);
}

static void
update_angle(float angle)
{
    int i;
    for (i = 0; i < GEARS; i++) {
        gears[i].angle = angle * gears[i].angle_rate + gears[i].angle_adjust;
    }
}

static void
gl_core_draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int i;

    for (i = 0; i < GEARS; i++) {
        glUniform1f(uniforms.tooth_angle.loc, M_PI / (gears[i].teeth / 2.0));
        glUniform1f(uniforms.gear_angle.loc, gears[i].angle);
        glUniform4fv(uniforms.gear_color.loc, 1, gears[i].color);
        glUniformMatrix4fv(uniforms.model.loc, 1, GL_FALSE,
                           (void*)gears[i].model);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, gears[i].vertex_buf_offset,
                              gears[i].num_vertices, gears[i].teeth);
    }

    gl_swapbuffers();
}

static void
gl_core_destruct()
{
}

struct gears_drawer gl32_core_drawer = {
    .name = "OpenGL 3.2 Core Profile Gears",
    .api_type = API_OPENGL_CORE,
    .set_window_attributes = set_gl_core_attributes,
    .set_global_state = set_gl_core_state,
    .resize = win_resize,
    .update_angle = update_angle,
    .draw = gl_core_draw,
    .destruct = gl_core_destruct,
};
