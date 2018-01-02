/* Jordan Justen : gears3d is public domain */

#include "gif.h"
#include "main.h"
#include "sim.h"

#include <math.h>

static GifWriter gw = { NULL, };

bool
start_gif(const char *filename)
{
    FILE *f = fopen(filename, "w");
    int delay = (int)roundf(sim_fixed_step_time);
    if (!delay)
        delay = 1;
    if (f)
        GifBegin(&gw, f, sim_width, sim_height, delay, false, NULL);
    return f != NULL;
}

void add_gif_frame(const void *rgba8, uint64_t delay_cs)
{
    if (gw.f == NULL || rgba8 == NULL)
        return;

    if (!delay_cs)
        delay_cs = 1;

    GifWriteFrame(&gw, rgba8, sim_width, sim_height, delay_cs, 8, true);
}

void end_gif(void)
{
    if (gw.f == NULL)
        return;

    GifEnd(&gw);
}
