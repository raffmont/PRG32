#include "prg32.h"
#include <stdio.h>

void prg32_debug_overlay_draw(int enabled,
                              int x,
                              int y,
                              uint32_t input_mask,
                              uint32_t frame) {
    if (!enabled) {
        return;
    }

    char line[80];
    snprintf(line,
             sizeof(line),
             "X:%d Y:%d IN:%04lx F:%lu T:%lu",
             x,
             y,
             (unsigned long)(input_mask & 0xffffu),
             (unsigned long)frame,
             (unsigned long)prg32_ticks_ms());

    prg32_gfx_rect(0, 0, PRG32_GAME_W, 8, PRG32_COLOR_BLACK);
    prg32_gfx_text8(0, 0, line, PRG32_COLOR_YELLOW, PRG32_COLOR_BLACK);
}
