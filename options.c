/* Jordan Justen : gears3d is public domain */

#include "main.h"
#include "sim.h"
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
    OPT_GL_ES,
    OPT_OUTPUT_FILE,
    OPT_VK,
    OPT_VSYNC,
    OPT_MAX_FRAMES,
    OPT_MAX_TIME,
    OPT_SIM_TIME,
    OPT_SPEED,
    OPT_WINSIZE,
    OPT_WINSYS,
};


static bool
str_to_float(const char *str, float *result)
{
    char *endptr;
    float res = strtof(str, &endptr);

    if (*endptr != '\0' || res <= 0.0f) {
        return false;
    } else {
        *result = res;
        return true;
    }
}

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
           LN("  --gles                run with OpenGLES")
           LN("  --max-frames=n        quit after `n` frames are drawn")
           LN("  --max-time=ms         quit after `ms` milliseconds")
           LN("  --output-file=fn      save to file `fn` (ppm supported)")
           LN("  --sim-time=ms         sim frame time in milliseconds (default is wall time)")
           LN("  --speed=dps           gear speed in degrees per second (default is 70)")
           LN("  --vk                  run with Vulkan")
           LN("  --vsync               run syncronized with monitor refresh")
           LN("  --win-size=WxH        `W` and `H` are the window size (default 300x300)")
           LN("  --winsys=w            where `w` is gbm, wayland or x11")
           LN("  -h, --help            display help message and exit"));
}

static bool
set_api_type(enum api_type api_type)
{
    if (gears_options.api_type == api_type) {
        return true;
    } else if (gears_options.api_type == API_NOT_SET) {
        gears_options.api_type = api_type;
        return true;
    } else {
        printf("Multiple 3D API types were requested!\n\n");
        return false;
    }
}

static bool
set_output_file(const char *filename)
{
    const char *ext = strrchr(filename, '.');
    if (!ext || ext[1] == '\0') {
        printf("No output file extension found!\n\n");
        return false;
    }

    if (strcmp(ext, ".ppm") == 0) {
        gears_options.output_type = OUTPUT_PPM;
    }

    if (gears_options.output_type != OUTPUT_NONE) {
        gears_options.output_file = filename;
        gears_options.extension_offset = (uint32_t)(ext - filename);
        return true;
    } else {
        printf("Output file extension (%s) not supported!\n\n", ext + 1);
        return false;
    }
}

static bool
set_winsize(const char *winsize_str)
{
    char tmp[20], *scan;
    uint64_t fst, snd;

    if (strlen(winsize_str) >= sizeof(tmp)) {
        printf("Unsupported win-size string length > %d\n\n",
               (int)sizeof(tmp) - 1);
        return false;
    }

    strcpy(tmp, winsize_str);
    scan = strchr(tmp, 'x');
    if (scan == NULL) {
        printf("win-size string doesn't contain 'x' character\n\n");
        return false;
    }
    *scan = '\0';

    if (!str_to_uint64(tmp, &fst)) {
        printf("win-size width isn't valid\n\n");
        return false;
    }

    if (!str_to_uint64(scan + 1, &snd)) {
        printf("win-size height isn't valid\n\n");
        return false;
    }

    gears_options.win_width = fst;
    gears_options.win_height = snd;
    return true;
}

static bool
set_winsys(const char *winsys_str)
{
    enum winsys_type winsys_type;

    if (strcmp(winsys_str, "gbm") == 0) {
        winsys_type = WINSYS_GBM;
    } else if (strcmp(winsys_str, "wayland") == 0) {
        winsys_type = WINSYS_WAYLAND;
    } else if (strcmp(winsys_str, "x11") == 0) {
        winsys_type = WINSYS_X11;
    } else {
        printf("Unknown winsys type: %s\n\n", winsys_str);
        return false;
    }

    if (gears_options.winsys_type == winsys_type) {
        return true;
    } else if (gears_options.winsys_type == WINSYS_AUTO) {
        gears_options.winsys_type = winsys_type;
        return true;
    } else {
        printf("Multiple winsys types were requested!\n\n");
        return false;
    }
}

bool
parse_options(int argc, char **argv)
{
    static const struct optparse_long longopts[] = {
        { "help",               OPT_HELP,               OPTPARSE_NONE },
        { "gl-compat",          OPT_GL_COMPAT,          OPTPARSE_NONE },
        { "gl-core",            OPT_GL_CORE,            OPTPARSE_NONE },
        { "gles",               OPT_GL_ES,              OPTPARSE_NONE },
        { "max-frames",         OPT_MAX_FRAMES,         OPTPARSE_REQUIRED },
        { "max-time",           OPT_MAX_TIME,           OPTPARSE_REQUIRED },
        { "output-file",        OPT_OUTPUT_FILE,        OPTPARSE_REQUIRED },
        { "sim-time",           OPT_SIM_TIME,           OPTPARSE_REQUIRED },
        { "speed",              OPT_SPEED,              OPTPARSE_REQUIRED },
        { "vk",                 OPT_VK   ,              OPTPARSE_NONE },
        { "vsync",              OPT_VSYNC,              OPTPARSE_NONE },
        { "win-size",           OPT_WINSIZE,            OPTPARSE_REQUIRED },
        { "winsys",             OPT_WINSYS,             OPTPARSE_REQUIRED },
        { 0 },
    };
    struct optparse options;
    int option;

    (void) optparse_arg; /* Silence compiler warning for unused function */

    memset(&gears_options, 0, sizeof(gears_options));
    gears_options.output_type = OUTPUT_NONE;
    gears_options.speed = 70; /* degrees per second */
    gears_options.sim_time = 0.0f;
    gears_options.win_width = 300;
    gears_options.win_height = 300;

    optparse_init(&options, argv);
    while ((option = optparse_long(&options, longopts, NULL)) != -1) {
        bool ok = true;
        switch (option) {
        case OPT_HELP:
            print_help();
            exit(EXIT_SUCCESS);
            return false;
        case OPT_GL_COMPAT:
            ok = set_api_type(API_OPENGL_COMPAT);
            break;
        case OPT_GL_CORE:
            ok = set_api_type(API_OPENGL_CORE);
            break;
        case OPT_GL_ES:
            ok = set_api_type(API_OPENGL_ES2);
            break;
        case OPT_MAX_FRAMES:
            ok = str_to_uint64(options.optarg, &gears_options.max_frames);
            break;
        case OPT_MAX_TIME:
            ok = str_to_uint64(options.optarg, &gears_options.max_time_ms);
            break;
        case OPT_OUTPUT_FILE:
            ok = set_output_file(options.optarg);
            break;
        case OPT_SIM_TIME:
            ok = str_to_float(options.optarg, &gears_options.sim_time);
            break;
        case OPT_SPEED:
            ok = str_to_uint64(options.optarg, &gears_options.speed);
            break;
        case OPT_VK:
            ok = set_api_type(API_VULKAN);
            break;
        case OPT_VSYNC:
            gears_options.vsync = true;
            break;
        case OPT_WINSIZE:
            ok = set_winsize(options.optarg);
            break;
        case OPT_WINSYS:
            ok = set_winsys(options.optarg);
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

    sim_width = gears_options.win_width;
    sim_height = gears_options.win_height;

    return true;
}
