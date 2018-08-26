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
    float e[] = { 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, -0.01667f, 0.0f, 0.0f, 0.0f,
                  -0.0f, -1.051282f, -1.0f, 0.0f, 0.0f, -10.256411f, 0.0f };
    for (int i = 0; i < 16; i++) {
        CHECK_FLOAT(f[i], e[i]);
    }
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
    float e[] = { 4.58f, -22.889999f, 18.999998f, 15.910001f,
                  26.129997f, 42.259998f, -38.699997f, -64.940002f,
                  26.409998f, 73.960007f, -91.300003f, -66.340012f,
                  -11.750001f, 39.0f, -84.720001f, 48.48f };
    for (int i = 0; i < 16; i++) {
        CHECK_FLOAT(d[i], e[i]);
    }
    return result;
}

static bool test_translate()
{
    bool result = true;
    float f[16];
    translate(f, -1.0, 2.0, 3.0);
    float e[] = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                  0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 2.0f, 3.0f, 1.0f };
    for (int i = 0; i < 16; i++) {
        CHECK_FLOAT(f[i], e[i]);
    }
    return result;
}

int main(int argc, char **argv)
{
    bool result = true;
    CHECK_BOOL(test_frustum());
    CHECK_BOOL(test_mult_m4m4());
    CHECK_BOOL(test_translate());
    return result ? 0 : 1;
}
