/* Gears. Draw them. In 3D.
 *
 * Jordan Justen : gears3d is public domain
 */

#include "main.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "optparse.h"

struct gears_options gears_options;

enum long_option_values {
    OPT_HELP = 'h',
    OPT_GL_CORE = 0x10000,
    OPT_GL_COMPAT,
    OPT_VSYNC,
    OPT_MAX_TIME,
};

static bool
str_to_uint64(const char *str, uint64_t *result)
{
    uint64_t value, mult10;
    static const uint64_t max_div_10 = UINT64_MAX / 10;

    if (str == NULL || *str == '\0' || result == NULL)
        return false;

    value = 0;
    do {
        if (!isdigit(*str))
            return false;
        uint8_t ones = *str - '0';
        if (value > max_div_10 - ones)
            return false;
        mult10 = 10 * value;
        if (UINT64_MAX - ones < mult10)
            return false;
        value = mult10 + ones;
        str++;
    } while (*str != '\0');

    *result = value;
    return true;
}

static void
print_help(void)
{
#define LN(s) s "\n"
    printf(LN("usage: gears3d [options]")
           LN("")
           LN("optional arguments:")
           LN("  --gl-compat           run with OpenGL compatibility profile")
           LN("  --gl-core             run with OpenGL core profile")
           LN("  --max-time=ms         run for specified number of milliseconds")
           LN("  --vsync               run syncronized with monitor refresh")
           LN("  -h, --help            display help message and exit"));
}

bool
parse_options(int argc, char **argv)
{
    static const struct optparse_long longopts[] = {
        { "help",               OPT_HELP,               OPTPARSE_NONE },
        { "gl-compat",          OPT_GL_COMPAT,          OPTPARSE_NONE },
        { "gl-core",            OPT_GL_CORE,            OPTPARSE_NONE },
        { "max-time",           OPT_MAX_TIME,           OPTPARSE_REQUIRED },
        { "vsync",              OPT_VSYNC,              OPTPARSE_NONE },
        { 0 },
    };
    struct optparse options;
    int option;

    (void) optparse_arg; /* Silence compiler warning for unused function */

    memset(&gears_options, 0, sizeof(gears_options));

    optparse_init(&options, argv);
    while ((option = optparse_long(&options, longopts, NULL)) != -1) {
        bool ok = true;
        switch (option) {
        case OPT_HELP:
            print_help();
            exit(EXIT_SUCCESS);
            return false;
        case OPT_GL_COMPAT:
            gears_options.compat = true;
            break;
        case OPT_GL_CORE:
            gears_options.core = true;
            break;
        case OPT_MAX_TIME:
            ok = str_to_uint64(options.optarg, &gears_options.max_time_ms);
            break;
        case OPT_VSYNC:
            gears_options.vsync = true;
            break;
        case '?':
            printf("Unknown option!\n\n");
            ok = false;
            break;
        }

        if (!ok) {
            print_help();
            return false;
        }
    }

    return true;
}
