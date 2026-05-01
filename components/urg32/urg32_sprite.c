#include "urg32.h"

int urg32_sprite_hitbox(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh) {
    if (aw <= 0 || ah <= 0 || bw <= 0 || bh <= 0) return 0;
    if (ax + aw <= bx) return 0;
    if (bx + bw <= ax) return 0;
    if (ay + ah <= by) return 0;
    if (by + bh <= ay) return 0;
    return 1;
}

void urg32_sprite_draw_8x8(int x, int y, const uint8_t *bits, uint16_t fg, uint16_t bg) {
    if (!bits) return;
    for (int row=0; row<8; ++row) for (int col=0; col<8; ++col) {
        uint16_t c = (bits[row] & (1 << (7-col))) ? fg : bg;
        urg32_gfx_pixel(x+col, y+row, c);
    }
}

void urg32_sprite_draw_16x16(int x, int y, const uint16_t *rgb565) {
    if (!rgb565) return;
    for (int row=0; row<16; ++row) for (int col=0; col<16; ++col) {
        uint16_t c = rgb565[row*16+col];
        if (c != 0xffff) urg32_gfx_pixel(x+col, y+row, c);
    }
}
