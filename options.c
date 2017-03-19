/* Gears. Draw them. In 3D.
 *
 * Jordan Justen : gears3d is public domain
 */

#include "main.h"
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
};

static void
print_help(void)
{
#define LN(s) s "\n"
    printf(LN("usage: gears3d [options]")
           LN("")
           LN("optional arguments:")
           LN("  --gl-compat           run with OpenGL compatibility profile")
           LN("  --gl-core             run with OpenGL core profile")
           LN("  -h, --help            display help message and exit"));
}

bool
parse_options(int argc, char **argv)
{
    static const struct optparse_long longopts[] = {
        { "help",               OPT_HELP,               OPTPARSE_NONE },
        { "gl-compat",          OPT_GL_COMPAT,          OPTPARSE_NONE },
        { "gl-core",            OPT_GL_CORE,            OPTPARSE_NONE },
        { 0 },
    };
    struct optparse options;
    int option;

    (void) optparse_arg; /* Silence compiler warning for unused function */

    memset(&gears_options, 0, sizeof(gears_options));

    optparse_init(&options, argv);
    while ((option = optparse_long(&options, longopts, NULL)) != -1) {
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
        case '?':
            printf("Unknown option!\n\n");
            print_help();
            return false;
        }
    }

    return true;
}
