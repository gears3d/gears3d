/* Gears. Draw them. In 3D.
 *
 * Jordan Justen : gears3d is public domain
 */

#include "g_math.h"
#include <math.h>
#include <assert.h>
#include <string.h>

static inline float*
mat4_el(float *mat4, int row, int column)
{
    /* column major */
    return mat4 + (row + (column << 2));
}

void
mult_m4m4(float *dest, float *src1, float *src2)
{
    int i, j;
    float mat4[16];

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            *mat4_el(mat4, i, j) =
                *mat4_el(src1, i, 0) * *mat4_el(src2, 0, j) +
                *mat4_el(src1, i, 1) * *mat4_el(src2, 1, j) +
                *mat4_el(src1, i, 2) * *mat4_el(src2, 2, j) +
                *mat4_el(src1, i, 3) * *mat4_el(src2, 3, j);
        }
    }

    memcpy(dest, mat4, sizeof(mat4));
}

void
copy_m4m3(mat3 *dst, mat4 *src)
{
    memcpy(dst, src, 3 * sizeof(float));
    memcpy(&dst[1], &src[1], 3 * sizeof(float));
    memcpy(&dst[2], &src[2], 3 * sizeof(float));
}

void
copy_m4m4x3(mat4x3 *dst, mat4 *src)
{
    memcpy(dst, src, 3 * sizeof(float));
    memcpy(&dst[1], &src[1], 3 * sizeof(float));
    memcpy(&dst[2], &src[2], 3 * sizeof(float));
    memcpy(&dst[3], &src[3], 3 * sizeof(float));
}

void
frustum(float *mat4, double left, double right, double bottom, double top,
        double nearVal, double farVal)
{
    /* https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glFrustum.xml
     *
     * perspective matrix:
     *
     * /   2 * nearVal                                \
     * |  ------------       0            A      0    |
     * |  right - left                                |
     * |                                              |
     * |                 2 * nearVal                  |
     * |       0        ------------      B      0    |
     * |                top - bottom                  |
     * |                                              |
     * |       0             0            C      D    |
     * |                                              |
     * |       0             0           -1      0    |
     * \                                              /
     *
     *     right + left
     * A = ------------
     *     right - left
     *
     *     top + bottom
     * B = ------------
     *     top - bottom
     *
     *       farVal + nearVal
     * C = - ----------------
     *       farVal - nearVal
     *
     *       2 * farVal * nearVal
     * D = - --------------------
     *         farVal - nearVal
     *
     */
    assert(left != right && top != bottom && farVal != nearVal);
    double A = (right + left) / (right - left);
    double B = (top + bottom) / (top - bottom);
    double C = - (farVal + nearVal) / (farVal - nearVal);
    double D = - (2.0 * farVal * nearVal) / (farVal - nearVal);
    memset(mat4, 0, sizeof(*mat4) * 4 * 4);
    *mat4_el(mat4, 0, 0) = (2.0 * nearVal) / (right - left);
    *mat4_el(mat4, 0, 2) = A;
    *mat4_el(mat4, 1, 1) = (2.0 * nearVal) / (top - bottom);
    *mat4_el(mat4, 1, 2) = B;
    *mat4_el(mat4, 2, 2) = C;
    *mat4_el(mat4, 2, 3) = D;
    *mat4_el(mat4, 3, 2) = -1.0;
}

void
perspective(float *mat4, double fovy, double aspect, double zNear, double zFar)
{
    /* https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml
     *
     * f = cotangent(fovy / 2)
     *
     * perspective matrix:
     *
     * /    f                                                  \
     * |  ------       0          0                 0          |
     * |  aspect                                               |
     * |                                                       |
     * |    0          f          0                 0          |
     * |                                                       |
     * |                     zFar + zNear    2 * zFar * zNear  |
     * |    0          0    --------------  ------------------ |
     * |                     zNear - zFar      zNear - zFar    |
     * |                                                       |
     * |    0          0         -1                 0          |
     * \                                                       /
     *
     */
    double f = 1.0 / tan(fovy / 2.0);
    memset(mat4, 0, sizeof(*mat4) * 4 * 4);
    assert(aspect != 0.0 && zNear != zFar);
    *mat4_el(mat4, 0, 0) = f / aspect;
    *mat4_el(mat4, 1, 1) = f;
    *mat4_el(mat4, 2, 2) = (zFar + zNear) / (zNear - zFar);
    *mat4_el(mat4, 2, 3) = (2.0 * zFar * zNear) / (zNear - zFar);
    *mat4_el(mat4, 3, 2) = -1.0;
}

void
rotate(float *mat4, double angle, double x, double y, double z)
{
    /* https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glRotate.xml
     *
     * perspective matrix:
     *
     * /                                                          \
     * | x^2*(1-c) + c     x*y*(1-c) - z*s   x*z*(1-c) + y*s   0  |
     * |                                                          |
     * | y*x*(1-c) + z*s   y^2*(1-c) + c     y*z*(1-c) - x*s   0  |
     * |                                                          |
     * | x*z*(1-c) - y*s   y*z*(1-c) + x*s   z^2*(1-c) + c     0  |
     * |                                                          |
     * |       0                  0                 0          1  |
     * \                                                          /
     *
     * c = cos(angle), s = sin(angle)
     *
     */
    double c = cos(angle);
    double omc = 1.0 - c;
    double s = sin(angle);
    memset(mat4, 0, sizeof(*mat4) * 4 * 4);
    *mat4_el(mat4, 0, 0) = (x * x * omc) + c;
    *mat4_el(mat4, 0, 1) = (x * y * omc) - z * s;
    *mat4_el(mat4, 0, 2) = (x * z * omc) + y * s;
    *mat4_el(mat4, 1, 0) = (y * x * omc) + z * s;
    *mat4_el(mat4, 1, 1) = (y * y * omc) + c;
    *mat4_el(mat4, 1, 2) = (y * z * omc) - x * s;
    *mat4_el(mat4, 2, 0) = (z * x * omc) - y * s;
    *mat4_el(mat4, 2, 1) = (z * y * omc) + x * s;
    *mat4_el(mat4, 2, 2) = (z * z * omc) + c;
    *mat4_el(mat4, 3, 3) = 1.0;
}

void
translate(float *mat4, float x, float y, float z)
{
    /* https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glTranslate.xml
     *
     * /              \
     * |  1  0  0  x  |
     * |              |
     * |  0  1  0  y  |
     * |              |
     * |  0  0  1  z  |
     * |              |
     * |  0  0  0  1  |
     * \              /
     *
     */
    memset(mat4, 0, sizeof(*mat4) * 4 * 4);
    *mat4_el(mat4, 0, 0) = 1.0;
    *mat4_el(mat4, 1, 1) = 1.0;
    *mat4_el(mat4, 2, 2) = 1.0;
    *mat4_el(mat4, 3, 3) = 1.0;
    *mat4_el(mat4, 0, 3) = x;
    *mat4_el(mat4, 1, 3) = y;
    *mat4_el(mat4, 2, 3) = z;
}
