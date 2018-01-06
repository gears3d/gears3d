/* Jordan Justen : gears3d is public domain */

#include "glfn.h"
#include "gl.h"
#include "main.h"
#include "sim.h"

#include <stdlib.h>
#include <string.h>

static void
gl_read_pixels(void *rgba8)
{
    glReadPixels(0, 0, sim_width, sim_height, GL_RGBA, GL_UNSIGNED_BYTE,
                 (GLvoid*)rgba8);
    const unsigned line_size = 4 * sim_width;
    void *swap_buf = malloc(line_size);
    uint32_t *pixels = (uint32_t*)rgba8;
    for (int row = 0; row < sim_width / 2; row++) {
        void *row1 = pixels + row * sim_width;
        void *row2 = pixels + (sim_height - 1 - row) * sim_width;
        memcpy(swap_buf, row1, line_size);
        memcpy(row1, row2, line_size);
        memcpy(row2, swap_buf, line_size);
    }
    free(swap_buf);
}

bool
gl_post_swap_buffer(void)
{
    if (!gears_options.output_file)
        return true;

    if (ready_for_new_frame()) {
        unsigned buf_size = 4 * sim_width * sim_height;
        void *rgba8 = malloc(buf_size);
        gl_read_pixels(rgba8);
        new_frame_data(rgba8);
        free(rgba8);
    }

    return true;
}
