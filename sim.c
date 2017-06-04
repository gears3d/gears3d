/* Jordan Justen : gears3d is public domain */

#include "sim.h"
#include <time.h>

static struct timespec start;
static uint64_t frame_count;

static uint64_t
delta_ms(const struct timespec *fst, const struct timespec *snd)
{
    return
        (snd->tv_sec - fst->tv_sec) * 1000 +
        (snd->tv_nsec - fst->tv_nsec) / 1000000;
}

void
init_sim(void)
{
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    frame_count = 0;
}

uint64_t
get_sim_time_ms(void)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);
    return delta_ms(&start, &now);
}

uint64_t
frame_drawn(void)
{
    return ++frame_count;
}

uint64_t
get_frame_count(void)
{
    return frame_count;
}
