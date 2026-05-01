#include "urg32.h"
#include <string.h>

typedef struct { uint8_t bits[8]; uint16_t fg; uint16_t bg; } tile_t;
static tile_t g_tiles[256];
static uint8_t g_map[URG32_TILE_ROWS][URG32_TILE_COLS];
static uint8_t g_dirty[URG32_TILE_ROWS][URG32_TILE_COLS];

void urg32_tile_clear(uint16_t color) {
    memset(g_map, 0, sizeof(g_map));
    memset(g_dirty, 1, sizeof(g_dirty));
    g_tiles[0].fg = color;
    g_tiles[0].bg = color;
}

void urg32_tile_define(uint8_t id, const uint8_t *bitmap8x8, uint16_t fg, uint16_t bg) {
    memcpy(g_tiles[id].bits, bitmap8x8, 8);
    g_tiles[id].fg = fg;
    g_tiles[id].bg = bg;
}

void urg32_tile_put(uint8_t tx, uint8_t ty, uint8_t id) {
    if (tx >= URG32_TILE_COLS || ty >= URG32_TILE_ROWS) return;
    if (g_map[ty][tx] == id) return;
    g_map[ty][tx] = id;
    g_dirty[ty][tx] = 1;
}

void urg32_tile_present(void) {
    for (int ty=0; ty<URG32_TILE_ROWS; ++ty) for (int tx=0; tx<URG32_TILE_COLS; ++tx) {
        if (!g_dirty[ty][tx]) continue;
        g_dirty[ty][tx] = 0;
        tile_t *t = &g_tiles[g_map[ty][tx]];
        for (int row=0; row<8; ++row) for (int col=0; col<8; ++col) {
            uint16_t c = (t->bits[row] & (1 << (7-col))) ? t->fg : t->bg;
            urg32_gfx_pixel(tx*8+col, ty*8+row, c);
        }
    }
    urg32_gfx_present();
}
