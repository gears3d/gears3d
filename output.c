/* Jordan Justen : gears3d is public domain */

#include "glfn.h"
#include "gl.h"
#include "main.h"
#include "sim.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint64_t this_frame_ms = 0;
static uint64_t last_frame_ms = 0;

static void save_ppm_file(const char *filename, const void *rgba8)
{
    FILE *f = fopen(filename, "w");
    if (f == NULL)
        return;
    fprintf(f, "P6\n%d %d\n255\x0a", sim_width, sim_height);
    const int pixels = sim_width * sim_height;
    for (int p = 0; p < pixels; p++) {
        fwrite(rgba8, 3, 1, f);
        rgba8 += 4;
    }
    fclose(f);
}

static void new_ppm_frame(const void *rgba8)
{
    assert(gears_options.output_file != NULL);
    int slen = strlen(gears_options.output_file) + 20;
    assert(slen > gears_options.extension_offset);
    char *filename = malloc(slen);
    if (filename == NULL)
        return;
    int len = snprintf(filename, slen, "%s%06lu",
                       gears_options.output_file, this_frame_ms);
    if (len > 0) {
        snprintf(filename + gears_options.extension_offset,
                 slen - gears_options.extension_offset,
                 "%06lu%s", this_frame_ms,
                 gears_options.output_file + gears_options.extension_offset);
        save_ppm_file(filename, rgba8);
    }
    free(filename);
}

bool init_output()
{
    switch(gears_options.output_type) {
    case OUTPUT_PPM:
    case OUTPUT_NONE:
        return true;
    default:
        return false;
    }
}

void end_output()
{
    switch(gears_options.output_type) {
    case OUTPUT_PPM:
    case OUTPUT_NONE:
    default:
        break;
    }
}

bool ready_for_new_frame()
{
    this_frame_ms = get_sim_time_ms();
    if (gears_options.output_type != OUTPUT_NONE && get_frame_count() == 0) {
        return true;
    }
    switch(gears_options.output_type) {
    case OUTPUT_PPM:
        return this_frame_ms > last_frame_ms;
    case OUTPUT_NONE:
    default:
        return false;
    }
}

void new_frame_data(const void *rgba8)
{
    switch(gears_options.output_type) {
    case OUTPUT_PPM:
        new_ppm_frame(rgba8);
        break;
    case OUTPUT_NONE:
    default:
        break;
    }
    last_frame_ms = this_frame_ms;
}
