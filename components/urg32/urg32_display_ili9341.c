#include "urg32.h"
#include "urg32_config.h"
#include <string.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_timer.h"

static spi_device_handle_t g_lcd;
static uint16_t g_fb[URG32_GAME_W * URG32_GAME_H];
static int g_dirty_x0, g_dirty_y0, g_dirty_x1, g_dirty_y1;
static const uint8_t g_font8[96][8] = { {0} };

static void dirty_reset(void) { g_dirty_x0 = 9999; g_dirty_y0 = 9999; g_dirty_x1 = -1; g_dirty_y1 = -1; }
static void dirty_add(int x, int y, int w, int h) {
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > URG32_GAME_W) w = URG32_GAME_W - x;
    if (y + h > URG32_GAME_H) h = URG32_GAME_H - y;
    if (w <= 0 || h <= 0) return;
    if (x < g_dirty_x0) g_dirty_x0 = x;
    if (y < g_dirty_y0) g_dirty_y0 = y;
    if (x + w - 1 > g_dirty_x1) g_dirty_x1 = x + w - 1;
    if (y + h - 1 > g_dirty_y1) g_dirty_y1 = y + h - 1;
}

static void lcd_cmd(uint8_t cmd) {
    spi_transaction_t t = {0};
    gpio_set_level(URG32_PIN_LCD_DC, 0);
    t.length = 8; t.tx_buffer = &cmd;
    spi_device_polling_transmit(g_lcd, &t);
}

static void lcd_data(const void *data, int len) {
    spi_transaction_t t = {0};
    gpio_set_level(URG32_PIN_LCD_DC, 1);
    t.length = len * 8; t.tx_buffer = data;
    spi_device_polling_transmit(g_lcd, &t);
}

static void lcd_addr(int x0, int y0, int x1, int y1) {
    uint8_t d[4];
    y0 += 20; y1 += 20;
    lcd_cmd(0x2a); d[0]=x0>>8; d[1]=x0; d[2]=x1>>8; d[3]=x1; lcd_data(d,4);
    lcd_cmd(0x2b); d[0]=y0>>8; d[1]=y0; d[2]=y1>>8; d[3]=y1; lcd_data(d,4);
    lcd_cmd(0x2c);
}

void urg32_display_init(void) {
    spi_bus_config_t bus = {
        .mosi_io_num = URG32_PIN_LCD_MOSI,
        .miso_io_num = URG32_PIN_LCD_MISO,
        .sclk_io_num = URG32_PIN_LCD_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 320 * 40 * 2
    };
    spi_device_interface_config_t dev = {
        .clock_speed_hz = 40000000,
        .mode = 0,
        .spics_io_num = URG32_PIN_LCD_CS,
        .queue_size = 7
    };
    gpio_set_direction(URG32_PIN_LCD_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(URG32_PIN_LCD_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(URG32_PIN_LCD_BL, GPIO_MODE_OUTPUT);
    spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_CH_AUTO);
    spi_bus_add_device(SPI2_HOST, &dev, &g_lcd);
    gpio_set_level(URG32_PIN_LCD_RST, 0); esp_rom_delay_us(20000);
    gpio_set_level(URG32_PIN_LCD_RST, 1); esp_rom_delay_us(120000);
    lcd_cmd(0x01); esp_rom_delay_us(150000);
    lcd_cmd(0x11); esp_rom_delay_us(120000);
    uint8_t pix = 0x55; lcd_cmd(0x3a); lcd_data(&pix,1);
    uint8_t mad = 0x28; lcd_cmd(0x36); lcd_data(&mad,1);
    lcd_cmd(0x29);
    gpio_set_level(URG32_PIN_LCD_BL, 1);
    dirty_reset();
}

uint32_t urg32_ticks_ms(void) { return (uint32_t)(esp_timer_get_time() / 1000u); }

void urg32_gfx_clear(uint16_t color) {
    for (int i = 0; i < URG32_GAME_W * URG32_GAME_H; ++i) g_fb[i] = color;
    dirty_add(0,0,URG32_GAME_W,URG32_GAME_H);
}

void urg32_gfx_pixel(int x, int y, uint16_t color) {
    if ((unsigned)x >= URG32_GAME_W || (unsigned)y >= URG32_GAME_H) return;
    g_fb[y * URG32_GAME_W + x] = color;
    dirty_add(x,y,1,1);
}

void urg32_gfx_rect(int x, int y, int w, int h, uint16_t color) {
    for (int yy = 0; yy < h; ++yy) for (int xx = 0; xx < w; ++xx) {
        int px=x+xx, py=y+yy;
        if ((unsigned)px < URG32_GAME_W && (unsigned)py < URG32_GAME_H) g_fb[py*URG32_GAME_W+px]=color;
    }
    dirty_add(x,y,w,h);
}

void urg32_gfx_text8(int x, int y, const char *s, uint16_t fg, uint16_t bg) {
    while (*s) {
        unsigned ch = (unsigned char)*s++;
        if (ch < 32 || ch > 127) ch='?';
        for (int row=0; row<8; ++row) {
            uint8_t bits = g_font8[ch-32][row];
            for (int col=0; col<8; ++col) urg32_gfx_pixel(x+col,y+row,(bits&(1<<(7-col)))?fg:bg);
        }
        x += 8;
    }
}

void urg32_gfx_present(void) {
    if (g_dirty_x1 < g_dirty_x0) return;
    int x0=g_dirty_x0, y0=g_dirty_y0, x1=g_dirty_x1, y1=g_dirty_y1;
    lcd_addr(x0,y0,x1,y1);
    for (int y=y0; y<=y1; ++y) lcd_data(&g_fb[y*URG32_GAME_W+x0], (x1-x0+1)*2);
    dirty_reset();
}
