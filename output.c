/* Jordan Justen : gears3d is public domain */

#include "glfn.h"
#include "gl.h"
#include "main.h"
#include "sim.h"

static uint64_t this_frame_ms = 0;
static uint64_t last_frame_ms = 0;

bool init_output()
{
    switch(gears_options.output_type) {
    case OUTPUT_NONE:
        return true;
    default:
        return false;
    }
}

void end_output()
{
    switch(gears_options.output_type) {
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
    case OUTPUT_NONE:
    default:
        return false;
    }
}

void new_frame_data(const void *rgba8)
{
    switch(gears_options.output_type) {
    case OUTPUT_NONE:
    default:
        break;
    }
    last_frame_ms = this_frame_ms;
}
