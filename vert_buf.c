/* Gears. Draw them. In 3D.
 *
 * Jordan Justen : gears3d is public domain
 */

#include <assert.h>
#include <math.h>
#include "vert_buf.h"

static void
add_vert(struct gear_vert **next_vert, GLfloat x, GLfloat y, GLfloat z,
         const struct vec3 *normal)
{
    assert(next_vert != NULL && *next_vert != NULL);
    (*next_vert)->pos.x = x;
    (*next_vert)->pos.y = y;
    (*next_vert)->pos.z = z;

    (*next_vert)->norm.x = normal->x;
    (*next_vert)->norm.y = normal->y;
    (*next_vert)->norm.z = normal->z;
    (*next_vert)++;
}

static void
add_vert_mult_xy_norm(struct gear_vert **next_vert, GLfloat x, GLfloat y,
                      GLfloat z, float mult)
{
    struct vec3 norm_tmp = { mult * x - x, mult * y - y, 0.0 };
    add_vert(next_vert, x, y, z, &norm_tmp);
}

#define TOOTH_VERTS 40

static void
tooth(struct gear_vert *verts, GLfloat inner_radius, GLfloat outer_radius,
      GLfloat width, GLint teeth, GLfloat tooth_depth, int tooth_num)
{
    const GLfloat r0 = inner_radius;
    const GLfloat r1 = outer_radius - tooth_depth / 2.0;
    const GLfloat r2 = outer_radius + tooth_depth / 2.0;
    const double da = M_PI / teeth / 2.0;
    const double pta = 2.0 * M_PI / teeth;
    const double ta = tooth_num * pta;
    const GLfloat half_width = 0.5 * width;
    const double dcos[] = { cos(ta), cos(ta + da), cos(ta + 2 * da),
                            cos(ta + 3 * da), cos(ta + 4 * da) };
    const double dsin[] = { sin(ta), sin(ta + da), sin(ta + 2 * da),
                            sin(ta + 3 * da), sin(ta + 4 * da) };
    struct gear_vert *nv = verts;
    const bool last_tooth = tooth_num < 0 || tooth_num == teeth - 1;
    assert(tooth_num >= -1 && tooth_num < teeth);

    static const struct vec3 pos_z = { 0.0, 0.0, 1.0 };
    static const struct vec3 neg_z = { 0.0, 0.0, -1.0 };
    struct vec3 norm_tmp;

    /* front face of tooth */
    add_vert(&nv, dcos[1] * r2, dsin[1] * r2, half_width, &pos_z);
    add_vert(&nv, dcos[2] * r2, dsin[2] * r2, half_width, &pos_z);
    add_vert(&nv, dcos[0] * r1, dsin[0] * r1, half_width, &pos_z);
    add_vert(&nv, dcos[3] * r1, dsin[3] * r1, half_width, &pos_z);

    /* front face of gear */
    add_vert(&nv, dcos[4] * r1, dsin[4] * r1, half_width, &pos_z);
    add_vert(&nv, dcos[4] * r1, dsin[4] * r1, half_width, &pos_z);
    add_vert(&nv, dcos[0] * r1, dsin[0] * r1, half_width, &pos_z);
    add_vert(&nv, dcos[4] * r0, dsin[4] * r0, half_width, &pos_z);
    add_vert(&nv, dcos[0] * r0, dsin[0] * r0, half_width, &pos_z);

    /* inner cylinder */
    add_vert_mult_xy_norm(&nv, dcos[4] * r0, dsin[4] * r0, half_width, 0.5);
    add_vert_mult_xy_norm(&nv, dcos[0] * r0, dsin[0] * r0, half_width, 0.5);
    add_vert_mult_xy_norm(&nv, dcos[4] * r0, dsin[4] * r0, -half_width, 0.5);
    add_vert_mult_xy_norm(&nv, dcos[0] * r0, dsin[0] * r0, -half_width, 0.5);

    /* back face of gear (first 2 are degenerate to reset normal) */
    add_vert(&nv, dcos[4] * r0, dsin[4] * r0, -half_width, &neg_z);
    add_vert(&nv, dcos[0] * r0, dsin[0] * r0, -half_width, &neg_z);
    add_vert(&nv, dcos[4] * r1, dsin[4] * r1, -half_width, &neg_z);
    add_vert(&nv, dcos[0] * r1, dsin[0] * r1, -half_width, &neg_z);
    add_vert(&nv, dcos[3] * r1, dsin[3] * r1, -half_width, &neg_z);

    /* back face of tooth */
    add_vert(&nv, dcos[1] * r2, dsin[1] * r2, -half_width, &neg_z);
    add_vert(&nv, dcos[2] * r2, dsin[2] * r2, -half_width, &neg_z);

    /* two degenerate triangles to jump to drawing the outer edge of gear */
    add_vert_mult_xy_norm(&nv, dcos[2] * r2, dsin[2] * r2, -half_width, 2.0);
    add_vert_mult_xy_norm(&nv, dcos[4] * r1, dsin[4] * r1, -half_width, 2.0);

    /* tooth recess outer edge */
    add_vert_mult_xy_norm(&nv, dcos[4] * r1, dsin[4] * r1, -half_width, 2.0);
    add_vert_mult_xy_norm(&nv, dcos[4] * r1, dsin[4] * r1, half_width, 2.0);
    add_vert_mult_xy_norm(&nv, dcos[3] * r1, dsin[3] * r1, -half_width, 2.0);
    add_vert_mult_xy_norm(&nv, dcos[3] * r1, dsin[3] * r1, half_width, 2.0);

    /* tooth leading edge (first 2 are degenerate to reset normal) */
    norm_tmp.x = dsin[3] * r1 - dsin[2] * r2;
    norm_tmp.y = -dcos[3] * r1 + dcos[2] * r2;
    norm_tmp.z = 0.0;
    add_vert(&nv, dcos[3] * r1, dsin[3] * r1, -half_width, &norm_tmp);
    add_vert(&nv, dcos[3] * r1, dsin[3] * r1, half_width, &norm_tmp);
    add_vert(&nv, dcos[2] * r2, dsin[2] * r2, -half_width, &norm_tmp);
    add_vert(&nv, dcos[2] * r2, dsin[2] * r2, half_width, &norm_tmp);

    /* tooth top edge (first 2 are degenerate to reset normal) */
    add_vert_mult_xy_norm(&nv, dcos[2] * r2, dsin[2] * r2, -half_width, 2.0);
    add_vert_mult_xy_norm(&nv, dcos[2] * r2, dsin[2] * r2, half_width, 2.0);
    add_vert_mult_xy_norm(&nv, dcos[1] * r2, dsin[1] * r2, -half_width, 2.0);
    add_vert_mult_xy_norm(&nv, dcos[1] * r2, dsin[1] * r2, half_width, 2.0);

    /* tooth trailing edge (first 2 are degenerate to reset normal) */
    norm_tmp.x = -dsin[0] * r1 + dsin[1] * r2;
    norm_tmp.y =  dcos[0] * r1 - dcos[1] * r2;
    norm_tmp.z = 0.0;
    add_vert(&nv, dcos[1] * r2, dsin[1] * r2, -half_width, &norm_tmp);
    add_vert(&nv, dcos[1] * r2, dsin[1] * r2, half_width, &norm_tmp);
    add_vert(&nv, dcos[0] * r1, dsin[0] * r1, -half_width, &norm_tmp);
    add_vert(&nv, dcos[0] * r1, dsin[0] * r1, half_width, &norm_tmp);

    if (!last_tooth) {
        /* two degenerate triangles to jump to drawing the next tooth */
        add_vert(&nv, dcos[0] * r1, dsin[0] * r1, half_width, &pos_z);
        add_vert(&nv, cos(ta + da + pta) * r2, sin(ta + da + pta) * r2,
                 half_width, &pos_z);
    }

    assert(nv - verts == (last_tooth ? TOOTH_VERTS - 2 : TOOTH_VERTS));
}

uint32_t
tooth_vertex_count(void)
{
    return TOOTH_VERTS - 2;
}

void
fill_tooth_vertices(struct gear_vert *buf, GLfloat inner_radius,
                    GLfloat outer_radius, GLfloat width, GLint teeth,
                    GLfloat tooth_depth)
{
    tooth(buf, inner_radius, outer_radius, width, teeth, tooth_depth, -1);
}

uint32_t
gear_vertex_count(uint32_t teeth)
{
    return (TOOTH_VERTS * teeth) - 2;
}

void
fill_gear_vertices(struct gear_vert *buf, GLfloat inner_radius,
                   GLfloat outer_radius, GLfloat width, GLint teeth,
                   GLfloat tooth_depth)
{
    int i;
    for (i = 0; i < teeth; i++) {
        tooth(buf + i * TOOTH_VERTS, inner_radius, outer_radius, width, teeth,
              tooth_depth, i);
    }
}
