#ifndef URG32_H
#define URG32_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define URG32_LCD_W 320
#define URG32_LCD_H 240
#define URG32_GAME_W 320
#define URG32_GAME_H 200
#define URG32_TEXT_COLS 40
#define URG32_TEXT_ROWS 25
#define URG32_TILE_W 8
#define URG32_TILE_H 8
#define URG32_TILE_COLS 40
#define URG32_TILE_ROWS 25

#define URG32_BTN_LEFT  (1u << 0)
#define URG32_BTN_RIGHT (1u << 1)
#define URG32_BTN_UP    (1u << 2)
#define URG32_BTN_DOWN  (1u << 3)
#define URG32_BTN_A     (1u << 4)
#define URG32_BTN_B     (1u << 5)
#define URG32_BTN_START (1u << 6)

#define URG32_COLOR_BLACK   0x0000
#define URG32_COLOR_WHITE   0xffff
#define URG32_COLOR_RED     0xf800
#define URG32_COLOR_GREEN   0x07e0
#define URG32_COLOR_BLUE    0x001f
#define URG32_COLOR_YELLOW  0xffe0
#define URG32_COLOR_CYAN    0x07ff
#define URG32_COLOR_MAGENTA 0xf81f

void urg32_init(void);
void urg32_set_mode(uint32_t mode);
uint32_t urg32_ticks_ms(void);
uint32_t urg32_input_read(void);
uint32_t urg32_controller_read(void);
const char *urg32_controller_name(uint32_t bit);
void urg32_audio_beep(uint32_t hz, uint32_t ms);

typedef struct {
    char game[24];
    char player[24];
    uint32_t score;
} urg32_score_t;
void urg32_wifi_scores_init(void);
void urg32_scores_api_start(void);
int urg32_score_submit(const char *game, const char *player, uint32_t score);

void urg32_console_clear(void);
void urg32_console_putc(int ch);
void urg32_console_write(const char *s);
void urg32_console_hex32(uint32_t value);

void urg32_gfx_clear(uint16_t color);
void urg32_gfx_present(void);
void urg32_gfx_pixel(int x, int y, uint16_t color);
void urg32_gfx_rect(int x, int y, int w, int h, uint16_t color);
void urg32_gfx_text8(int x, int y, const char *s, uint16_t fg, uint16_t bg);

void urg32_tile_clear(uint16_t color);
void urg32_tile_define(uint8_t id, const uint8_t *bitmap8x8, uint16_t fg, uint16_t bg);
void urg32_tile_put(uint8_t tx, uint8_t ty, uint8_t id);
void urg32_tile_present(void);

int urg32_sprite_hitbox(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh);
void urg32_sprite_draw_8x8(int x, int y, const uint8_t *bits, uint16_t fg, uint16_t bg);
void urg32_sprite_draw_16x16(int x, int y, const uint16_t *rgb565);

/* Assembly demos export per-game init/update/draw symbols selected by main. */

#ifdef __cplusplus
}
#endif
#endif
