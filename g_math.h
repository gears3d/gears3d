/* Jordan Justen : gears3d is public domain */

#ifndef __math_h_included__
#define __math_h_included__

#define ALIGN(x, a) (((x) % (a) == 0) ? (x) : ((x) + ((a) - ((x) % (a)))))

#include <stdbool.h>

#pragma pack(1)

struct vec2 {
    float x, y;
};

struct vec3 {
    float x, y, z;
};

struct vec4 {
    float x, y, z, w;
};

typedef struct vec3 mat3[3];
typedef struct vec3 mat4x3[4];
typedef struct vec4 mat4[4];

#pragma pack()

void mult_m4m4(float *dest, float *src1, float *src2);
void copy_m4m3(mat3 *dst, mat4 *src);
void copy_m4m4x3(mat4x3 *dst, mat4 *src);

void frustum(float *mat4, double left, double right, double bottom,
             double top, double nearVal, double farVal);
void perspective(float *mat4, double fovy, double aspect, double zNear,
                 double zFar);

void rotate(float *mat4, double angle, double x, double y, double z);
void translate(float *mat4, float x, float y, float z);

#endif
