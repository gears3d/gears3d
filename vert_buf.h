/* Gears. Draw them. In 3D.
 *
 * Jordan Justen : gears3d is public domain
 */

#ifndef __vert_buf_h_included__
#define __vert_buf_h_included__

#include <stdint.h>
#include <epoxy/gl.h>
#include "g_math.h"

#pragma pack(1)

struct gear_vert {
    struct vec3 pos;
    struct vec3 norm;
};

#pragma pack()

uint32_t tooth_vertex_count(void);
void fill_tooth_vertices(struct gear_vert *buf, GLfloat inner_radius,
                         GLfloat outer_radius, GLfloat width, GLint teeth,
                         GLfloat tooth_depth);

uint32_t gear_vertex_count(uint32_t teeth);
void fill_gear_vertices(struct gear_vert *buf, GLfloat inner_radius,
                        GLfloat outer_radius, GLfloat width, GLint teeth,
                        GLfloat tooth_depth);

#endif
