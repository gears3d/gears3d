/* Jordan Justen : gears3d is public domain */

#include "g_math.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#define CHECK_FLOAT(v, e)                                               \
    while (fabsf(e - v) > fabsf(0.01 * e)) {                            \
        fprintf(stderr, "compare failed: value:    %f (" #v ")\n"       \
                        "                expected: %f (" #e ")\n",      \
                v, e);                                                  \
        result = false;                                                 \
        break;                                                          \
    }

#define CHECK_BOOL(c)                                           \
    while (!(c)) {                                              \
        fprintf(stderr, "test failed: " #c "\n");               \
        result = false;                                         \
        break;                                                  \
    }

static bool test_frustum()
{
    bool result = true;
    float f[16];
    frustum(f, -1.0, 1.0, 300, -300, 5.0, 200.0);
    CHECK_FLOAT(f[0], 5.0f);
    CHECK_FLOAT(f[1], 0.0f);
    CHECK_FLOAT(f[2], 0.0f);
    CHECK_FLOAT(f[3], 0.0f);
    CHECK_FLOAT(f[4], 0.0f);
    CHECK_FLOAT(f[5], -0.01667f);
    CHECK_FLOAT(f[6], 0.0f);
    CHECK_FLOAT(f[7], 0.0f);
    CHECK_FLOAT(f[8], 0.0f);
    CHECK_FLOAT(f[9], -0.0f);
    CHECK_FLOAT(f[10], -1.051282f);
    CHECK_FLOAT(f[11], -1.0f);
    CHECK_FLOAT(f[12], 0.0f);
    CHECK_FLOAT(f[13], 0.0f);
    CHECK_FLOAT(f[14], -10.256411f);
    CHECK_FLOAT(f[15], 0.0f);
    return result;
}

static bool test_mult_m4m4()
{
    bool result = true;
    float d[16];
    float m1[] = { 0.1f, -0.5f, -1.0f, 2.5f, -0.5f, 3.0f, -10.f, 7.0f,
                   4.2f, 6.3f, -7.1f, -8.8f, 1.8f, -2.2f, 3.1f, -2.1f };
    float m2[] = { 5.3f, 0.3f, -1.4f, 5.6f, 3.3f, -2.4f, 7.1f, -2.9f,
                   -6.4f, 2.3f, 8.6f, -4.4f, 6.6f, 3.7f, 1.6f, -9.6f };
    mult_m4m4(d, m1, m2);
    CHECK_FLOAT(d[0], 4.58f);
    CHECK_FLOAT(d[1], -22.889999f);
    CHECK_FLOAT(d[2], 18.999998f);
    CHECK_FLOAT(d[3], 15.910001f);
    CHECK_FLOAT(d[4], 26.129997f);
    CHECK_FLOAT(d[5], 42.259998f);
    CHECK_FLOAT(d[6], -38.699997f);
    CHECK_FLOAT(d[7], -64.940002f);
    CHECK_FLOAT(d[8], 26.409998f);
    CHECK_FLOAT(d[9], 73.960007f);
    CHECK_FLOAT(d[10], -91.300003f);
    CHECK_FLOAT(d[11], -66.340012f);
    CHECK_FLOAT(d[12], -11.750001f);
    CHECK_FLOAT(d[13], 39.0f);
    CHECK_FLOAT(d[14], -84.720001f);
    CHECK_FLOAT(d[15], 48.48f);
    return result;
}

int main(int argc, char **argv)
{
    bool result = true;
    CHECK_BOOL(test_frustum());
    CHECK_BOOL(test_mult_m4m4());
    return result ? 0 : 1;
}
