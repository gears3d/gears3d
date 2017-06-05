/* Jordan Justen : gears3d is public domain */

#ifndef __sim_h_included__
#define __sim_h_included__

#include <stdbool.h>
#include <stdint.h>

void init_sim(void);
uint64_t get_sim_time_ms(void);
uint64_t frame_drawn(void);
uint64_t get_frame_count(void);

extern uint32_t sim_width, sim_height;
extern bool sim_done;

#endif
