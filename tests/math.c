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

int main(int argc, char **argv)
{
    bool result = true;
    CHECK_BOOL(test_frustum());
    return result ? 0 : 1;
}
