#include "urg32.h"
#include "urg32_config.h"
#include <stdio.h>
#include <string.h>
#include "driver/uart.h"

static uint32_t g_mode = URG32_DEFAULT_MODE;
static char g_text[URG32_TEXT_ROWS][URG32_TEXT_COLS];
static int g_cx = 0;
static int g_cy = 0;

void urg32_set_mode(uint32_t mode) {
    if (mode > URG32_MODE_UART_LCD_MIRROR) {
        mode = URG32_MODE_UART_LCD_MIRROR;
    }
    g_mode = mode;
}

static void console_redraw_char(int x, int y) {
    if ((unsigned)x >= URG32_TEXT_COLS || (unsigned)y >= URG32_TEXT_ROWS) {
        return;
    }
    char s[2] = { g_text[y][x], 0 };
    urg32_gfx_text8(x * 8, y * 8, s, URG32_COLOR_GREEN, URG32_COLOR_BLACK);
}

static void console_redraw_all(void) {
    for (int y = 0; y < URG32_TEXT_ROWS; ++y) {
        for (int x = 0; x < URG32_TEXT_COLS; ++x) {
            console_redraw_char(x, y);
        }
    }
}

static void console_scroll(void) {
    memmove(g_text[0], g_text[1], (URG32_TEXT_ROWS - 1) * URG32_TEXT_COLS);
    memset(g_text[URG32_TEXT_ROWS - 1], ' ', URG32_TEXT_COLS);
    g_cy = URG32_TEXT_ROWS - 1;
    if (g_mode != URG32_MODE_UART_ONLY) {
        console_redraw_all();
    }
}

void urg32_console_clear(void) {
    memset(g_text, ' ', sizeof(g_text));
    g_cx = 0;
    g_cy = 0;
    if (g_mode != URG32_MODE_UART_ONLY) {
        urg32_gfx_clear(URG32_COLOR_BLACK);
        urg32_gfx_present();
    }
}

void urg32_console_putc(int ch) {
    if (g_mode == URG32_MODE_UART_ONLY || g_mode == URG32_MODE_UART_LCD_MIRROR) {
        char c = (char)ch;
        uart_write_bytes(UART_NUM_0, &c, 1);
    }
    if (g_mode == URG32_MODE_UART_ONLY) return;
    if (ch == '\r') { g_cx = 0; return; }
    if (ch == '\n') { g_cx = 0; g_cy++; if (g_cy >= URG32_TEXT_ROWS) console_scroll(); return; }
    if (ch == '\b') { if (g_cx > 0) g_cx--; g_text[g_cy][g_cx] = ' '; console_redraw_char(g_cx, g_cy); return; }
    if (ch < 32 || ch > 126) ch = '?';
    g_text[g_cy][g_cx] = (char)ch;
    console_redraw_char(g_cx, g_cy);
    g_cx++;
    if (g_cx >= URG32_TEXT_COLS) { g_cx = 0; g_cy++; if (g_cy >= URG32_TEXT_ROWS) console_scroll(); }
}

void urg32_console_write(const char *s) {
    if (!s) {
        return;
    }
    while (*s) urg32_console_putc((unsigned char)*s++);
}

void urg32_console_hex32(uint32_t value) {
    char b[11];
    snprintf(b, sizeof(b), "0x%08lx", (unsigned long)value);
    urg32_console_write(b);
}
