#include "prg32.h"
#include "prg32_config.h"
#include <string.h>
#include "driver/gpio.h"
#ifndef PRG32_LCD_SOFT_SPI
#define PRG32_LCD_SOFT_SPI 0
#endif
#if !PRG32_LCD_SOFT_SPI
#include "driver/spi_master.h"
#endif
#include "esp_err.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "prg32_lcd";

#ifndef PRG32_LCD_SPI_CLOCK_HZ
#define PRG32_LCD_SPI_CLOCK_HZ 27000000
#endif

#ifndef PRG32_LCD_BACKLIGHT_ACTIVE_LEVEL
#define PRG32_LCD_BACKLIGHT_ACTIVE_LEVEL 1
#endif

#ifndef PRG32_LCD_MADCTL
#define PRG32_LCD_MADCTL 0x28
#endif

#ifndef PRG32_LCD_BOOT_TEST_MS
#define PRG32_LCD_BOOT_TEST_MS 0
#endif

#if !PRG32_LCD_SOFT_SPI
static spi_device_handle_t g_lcd;
#endif
#if PRG32_LCD_SOFT_SPI
static int g_lcd_soft_spi_ready;
#endif
static int g_lcd_ready;
static uint16_t g_fb[PRG32_GAME_W * PRG32_GAME_H];
static uint16_t g_line_buf[PRG32_GAME_W];
static int g_dirty_x0, g_dirty_y0, g_dirty_x1, g_dirty_y1;

static esp_err_t lcd_prepare_output_pin(int pin, const char *name) {
    if (pin < 0) {
        return ESP_OK;
    }
    esp_err_t err = gpio_reset_pin((gpio_num_t)pin);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LCD %s GPIO reset failed: %s", name, esp_err_to_name(err));
        return err;
    }
    err = gpio_set_pull_mode((gpio_num_t)pin, GPIO_FLOATING);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LCD %s GPIO pull setup failed: %s", name, esp_err_to_name(err));
        return err;
    }
    err = gpio_set_drive_capability((gpio_num_t)pin, GPIO_DRIVE_CAP_3);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LCD %s GPIO drive setup failed: %s", name, esp_err_to_name(err));
        return err;
    }
    err = gpio_set_direction((gpio_num_t)pin, GPIO_MODE_OUTPUT);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LCD %s GPIO direction failed: %s", name, esp_err_to_name(err));
    }
    return err;
}

static esp_err_t lcd_prepare_input_pin(int pin, const char *name) {
    if (pin < 0) {
        return ESP_OK;
    }
    esp_err_t err = gpio_reset_pin((gpio_num_t)pin);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LCD %s GPIO reset failed: %s", name, esp_err_to_name(err));
        return err;
    }
    err = gpio_set_pull_mode((gpio_num_t)pin, GPIO_FLOATING);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LCD %s GPIO pull setup failed: %s", name, esp_err_to_name(err));
        return err;
    }
    err = gpio_set_direction((gpio_num_t)pin, GPIO_MODE_INPUT);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LCD %s GPIO direction failed: %s", name, esp_err_to_name(err));
    }
    return err;
}

static const uint8_t g_font8[96][8] = {
    [' ' - 32] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    ['!' - 32] = {0x18,0x18,0x18,0x18,0x18,0x00,0x18,0x00},
    ['"' - 32] = {0x24,0x24,0x24,0x00,0x00,0x00,0x00,0x00},
    ['#' - 32] = {0x24,0x7e,0x24,0x24,0x7e,0x24,0x00,0x00},
    ['$' - 32] = {0x18,0x3e,0x58,0x3c,0x1a,0x7c,0x18,0x00},
    ['%' - 32] = {0x62,0x64,0x08,0x10,0x26,0x46,0x00,0x00},
    ['&' - 32] = {0x30,0x48,0x30,0x4a,0x44,0x3a,0x00,0x00},
    ['\'' - 32] = {0x18,0x18,0x10,0x00,0x00,0x00,0x00,0x00},
    ['(' - 32] = {0x0c,0x10,0x20,0x20,0x20,0x10,0x0c,0x00},
    [')' - 32] = {0x30,0x08,0x04,0x04,0x04,0x08,0x30,0x00},
    ['*' - 32] = {0x00,0x24,0x18,0x7e,0x18,0x24,0x00,0x00},
    ['+' - 32] = {0x00,0x18,0x18,0x7e,0x18,0x18,0x00,0x00},
    [',' - 32] = {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x10},
    ['-' - 32] = {0x00,0x00,0x00,0x7e,0x00,0x00,0x00,0x00},
    ['.' - 32] = {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00},
    ['/' - 32] = {0x02,0x04,0x08,0x10,0x20,0x40,0x00,0x00},
    ['0' - 32] = {0x3c,0x42,0x46,0x4a,0x52,0x62,0x3c,0x00},
    ['1' - 32] = {0x18,0x28,0x08,0x08,0x08,0x08,0x3e,0x00},
    ['2' - 32] = {0x3c,0x42,0x02,0x0c,0x30,0x40,0x7e,0x00},
    ['3' - 32] = {0x3c,0x42,0x02,0x1c,0x02,0x42,0x3c,0x00},
    ['4' - 32] = {0x0c,0x14,0x24,0x44,0x7e,0x04,0x04,0x00},
    ['5' - 32] = {0x7e,0x40,0x7c,0x02,0x02,0x42,0x3c,0x00},
    ['6' - 32] = {0x1c,0x20,0x40,0x7c,0x42,0x42,0x3c,0x00},
    ['7' - 32] = {0x7e,0x02,0x04,0x08,0x10,0x10,0x10,0x00},
    ['8' - 32] = {0x3c,0x42,0x42,0x3c,0x42,0x42,0x3c,0x00},
    ['9' - 32] = {0x3c,0x42,0x42,0x3e,0x02,0x04,0x38,0x00},
    [':' - 32] = {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x00},
    [';' - 32] = {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x10},
    ['<' - 32] = {0x0c,0x10,0x20,0x40,0x20,0x10,0x0c,0x00},
    ['=' - 32] = {0x00,0x00,0x7e,0x00,0x7e,0x00,0x00,0x00},
    ['>' - 32] = {0x30,0x08,0x04,0x02,0x04,0x08,0x30,0x00},
    ['?' - 32] = {0x3c,0x42,0x02,0x0c,0x10,0x00,0x10,0x00},
    ['@' - 32] = {0x3c,0x42,0x5a,0x56,0x5e,0x40,0x3c,0x00},
    ['A' - 32] = {0x18,0x24,0x42,0x42,0x7e,0x42,0x42,0x00},
    ['B' - 32] = {0x7c,0x42,0x42,0x7c,0x42,0x42,0x7c,0x00},
    ['C' - 32] = {0x3c,0x42,0x40,0x40,0x40,0x42,0x3c,0x00},
    ['D' - 32] = {0x78,0x44,0x42,0x42,0x42,0x44,0x78,0x00},
    ['E' - 32] = {0x7e,0x40,0x40,0x78,0x40,0x40,0x7e,0x00},
    ['F' - 32] = {0x7e,0x40,0x40,0x78,0x40,0x40,0x40,0x00},
    ['G' - 32] = {0x3c,0x42,0x40,0x4e,0x42,0x42,0x3c,0x00},
    ['H' - 32] = {0x42,0x42,0x42,0x7e,0x42,0x42,0x42,0x00},
    ['I' - 32] = {0x3e,0x08,0x08,0x08,0x08,0x08,0x3e,0x00},
    ['J' - 32] = {0x1e,0x04,0x04,0x04,0x44,0x44,0x38,0x00},
    ['K' - 32] = {0x42,0x44,0x48,0x70,0x48,0x44,0x42,0x00},
    ['L' - 32] = {0x40,0x40,0x40,0x40,0x40,0x40,0x7e,0x00},
    ['M' - 32] = {0x42,0x66,0x5a,0x5a,0x42,0x42,0x42,0x00},
    ['N' - 32] = {0x42,0x62,0x52,0x4a,0x46,0x42,0x42,0x00},
    ['O' - 32] = {0x3c,0x42,0x42,0x42,0x42,0x42,0x3c,0x00},
    ['P' - 32] = {0x7c,0x42,0x42,0x7c,0x40,0x40,0x40,0x00},
    ['Q' - 32] = {0x3c,0x42,0x42,0x42,0x4a,0x44,0x3a,0x00},
    ['R' - 32] = {0x7c,0x42,0x42,0x7c,0x48,0x44,0x42,0x00},
    ['S' - 32] = {0x3c,0x42,0x40,0x3c,0x02,0x42,0x3c,0x00},
    ['T' - 32] = {0x7e,0x08,0x08,0x08,0x08,0x08,0x08,0x00},
    ['U' - 32] = {0x42,0x42,0x42,0x42,0x42,0x42,0x3c,0x00},
    ['V' - 32] = {0x42,0x42,0x42,0x42,0x24,0x24,0x18,0x00},
    ['W' - 32] = {0x42,0x42,0x42,0x5a,0x5a,0x66,0x42,0x00},
    ['X' - 32] = {0x42,0x24,0x18,0x18,0x18,0x24,0x42,0x00},
    ['Y' - 32] = {0x42,0x24,0x18,0x08,0x08,0x08,0x08,0x00},
    ['Z' - 32] = {0x7e,0x02,0x04,0x18,0x20,0x40,0x7e,0x00},
    ['[' - 32] = {0x3c,0x20,0x20,0x20,0x20,0x20,0x3c,0x00},
    ['\\' - 32] = {0x40,0x20,0x10,0x08,0x04,0x02,0x00,0x00},
    [']' - 32] = {0x3c,0x04,0x04,0x04,0x04,0x04,0x3c,0x00},
    ['^' - 32] = {0x18,0x24,0x42,0x00,0x00,0x00,0x00,0x00},
    ['_' - 32] = {0x00,0x00,0x00,0x00,0x00,0x00,0x7e,0x00},
    ['`' - 32] = {0x20,0x10,0x08,0x00,0x00,0x00,0x00,0x00},
    ['{' - 32] = {0x0e,0x10,0x10,0x60,0x10,0x10,0x0e,0x00},
    ['|' - 32] = {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00},
    ['}' - 32] = {0x70,0x08,0x08,0x06,0x08,0x08,0x70,0x00},
    ['~' - 32] = {0x00,0x00,0x32,0x4c,0x00,0x00,0x00,0x00},
};

static void dirty_reset(void) {
    g_dirty_x0 = 9999;
    g_dirty_y0 = 9999;
    g_dirty_x1 = -1;
    g_dirty_y1 = -1;
}

static void dirty_add(int x, int y, int w, int h) {
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > PRG32_GAME_W) w = PRG32_GAME_W - x;
    if (y + h > PRG32_GAME_H) h = PRG32_GAME_H - y;
    if (w <= 0 || h <= 0) return;
    if (x < g_dirty_x0) g_dirty_x0 = x;
    if (y < g_dirty_y0) g_dirty_y0 = y;
    if (x + w - 1 > g_dirty_x1) g_dirty_x1 = x + w - 1;
    if (y + h - 1 > g_dirty_y1) g_dirty_y1 = y + h - 1;
}

static uint16_t rgb565_wire(uint16_t color) {
    return (uint16_t)((color << 8) | (color >> 8));
}

static const uint8_t *font_glyph(unsigned ch) {
    if (ch >= 'a' && ch <= 'z') {
        ch -= 'a' - 'A';
    }
    if (ch < 32 || ch > 126) {
        ch = '?';
    }
    return g_font8[ch - 32];
}

static void lcd_select(void) {
#if PRG32_PIN_LCD_CS >= 0
    gpio_set_level(PRG32_PIN_LCD_CS, 0);
#endif
}

static void lcd_deselect(void) {
#if PRG32_PIN_LCD_CS >= 0
    gpio_set_level(PRG32_PIN_LCD_CS, 1);
#endif
}

#if PRG32_LCD_SOFT_SPI
static esp_err_t lcd_soft_spi_init(void) {
    esp_err_t err = lcd_prepare_output_pin(PRG32_PIN_LCD_MOSI, "software SPI MOSI");
    if (err != ESP_OK) {
        return err;
    }
    err = lcd_prepare_output_pin(PRG32_PIN_LCD_SCLK, "software SPI SCLK");
    if (err != ESP_OK) {
        return err;
    }
    err = lcd_prepare_output_pin(PRG32_PIN_LCD_CS, "software SPI CS");
    if (err != ESP_OK) {
        return err;
    }
#if PRG32_PIN_LCD_MISO >= 0
    err = lcd_prepare_input_pin(PRG32_PIN_LCD_MISO, "software SPI MISO");
    if (err != ESP_OK) {
        return err;
    }
#endif
    lcd_deselect();
    gpio_set_level(PRG32_PIN_LCD_SCLK, 0);
    gpio_set_level(PRG32_PIN_LCD_MOSI, 0);
    g_lcd_soft_spi_ready = 1;
    return ESP_OK;
}

static void lcd_soft_spi_write_byte_raw(uint8_t byte) {
    for (int bit = 7; bit >= 0; --bit) {
        gpio_set_level(PRG32_PIN_LCD_SCLK, 0);
        gpio_set_level(PRG32_PIN_LCD_MOSI, (byte >> bit) & 1);
        esp_rom_delay_us(1);
        gpio_set_level(PRG32_PIN_LCD_SCLK, 1);
        esp_rom_delay_us(1);
    }
    gpio_set_level(PRG32_PIN_LCD_SCLK, 0);
}

static uint8_t lcd_soft_spi_read_byte_raw(void) {
    uint8_t byte = 0;
    for (int bit = 7; bit >= 0; --bit) {
        gpio_set_level(PRG32_PIN_LCD_SCLK, 0);
        gpio_set_level(PRG32_PIN_LCD_MOSI, 0);
        esp_rom_delay_us(1);
        gpio_set_level(PRG32_PIN_LCD_SCLK, 1);
        if (gpio_get_level(PRG32_PIN_LCD_MISO)) {
            byte |= (uint8_t)(1u << bit);
        }
        esp_rom_delay_us(1);
    }
    gpio_set_level(PRG32_PIN_LCD_SCLK, 0);
    return byte;
}

static void lcd_soft_spi_write_raw(const void *data, int len) {
    if (!data || len <= 0) {
        return;
    }
    if (!g_lcd_soft_spi_ready) {
        ESP_LOGE(TAG, "LCD software SPI write skipped before bus init");
        return;
    }
    const uint8_t *bytes = (const uint8_t *)data;
    for (int i = 0; i < len; ++i) {
        lcd_soft_spi_write_byte_raw(bytes[i]);
    }
}
#endif

#if !PRG32_LCD_SOFT_SPI
static int lcd_spi_write_raw(const void *data, int len) {
    if (!data || len <= 0) {
        return 1;
    }
    if (!g_lcd) {
        ESP_LOGE(TAG, "LCD SPI write skipped before device init");
        return 0;
    }
    spi_transaction_t t = {0};
    t.length = (size_t)len * 8;
    t.tx_buffer = data;
    esp_err_t err = spi_device_polling_transmit(g_lcd, &t);
    if (err != ESP_OK) {
        ESP_LOGE(TAG,
                 "LCD SPI write of %d bytes failed: %s",
                 len,
                 esp_err_to_name(err));
        return 0;
    }
    return 1;
}
#endif

static int lcd_write_raw(const void *data, int len) {
#if PRG32_LCD_SOFT_SPI
    lcd_soft_spi_write_raw(data, len);
    return 1;
#else
    return lcd_spi_write_raw(data, len);
#endif
}

static void lcd_cmd(uint8_t cmd) {
    gpio_set_level(PRG32_PIN_LCD_DC, 0);
    lcd_select();
    lcd_write_raw(&cmd, 1);
    lcd_deselect();
}

static void lcd_data(const void *data, int len) {
    if (!data || len <= 0) {
        return;
    }
    gpio_set_level(PRG32_PIN_LCD_DC, 1);
    lcd_select();
    lcd_write_raw(data, len);
    lcd_deselect();
}

static int lcd_read_command(uint8_t cmd, void *data, int len) {
    if (!data || len <= 0) {
        return 0;
    }
#if PRG32_LCD_SOFT_SPI
    if (!g_lcd_soft_spi_ready) {
        ESP_LOGE(TAG, "LCD software SPI read skipped before bus init");
        return 0;
    }
    lcd_select();
    gpio_set_level(PRG32_PIN_LCD_DC, 0);
    lcd_soft_spi_write_byte_raw(cmd);
    gpio_set_level(PRG32_PIN_LCD_DC, 1);
    uint8_t *bytes = (uint8_t *)data;
    for (int i = 0; i < len; ++i) {
        bytes[i] = lcd_soft_spi_read_byte_raw();
    }
    lcd_deselect();
    return 1;
#else
    if (!g_lcd) {
        ESP_LOGE(TAG, "LCD read skipped before SPI device init");
        return 0;
    }
    lcd_select();
    gpio_set_level(PRG32_PIN_LCD_DC, 0);
    if (!lcd_write_raw(&cmd, 1)) {
        lcd_deselect();
        return 0;
    }
    spi_transaction_t t = {0};
    gpio_set_level(PRG32_PIN_LCD_DC, 1);
    t.length = 0;
    t.rxlength = (size_t)len * 8;
    t.rx_buffer = data;
    esp_err_t err = spi_device_polling_transmit(g_lcd, &t);
    if (err != ESP_OK) {
        ESP_LOGE(TAG,
                 "LCD read of %d bytes failed: %s",
                 len,
                 esp_err_to_name(err));
        lcd_deselect();
        return 0;
    }
    lcd_deselect();
    return 1;
#endif
}

static void lcd_cmd_data(uint8_t cmd, const uint8_t *data, int len) {
    gpio_set_level(PRG32_PIN_LCD_DC, 0);
    lcd_select();
    lcd_write_raw(&cmd, 1);
    if (data && len > 0) {
        gpio_set_level(PRG32_PIN_LCD_DC, 1);
        lcd_write_raw(data, len);
    }
    lcd_deselect();
}

static void lcd_backlight(int on) {
#if PRG32_PIN_LCD_BL >= 0
    int level = PRG32_LCD_BACKLIGHT_ACTIVE_LEVEL ? 1 : 0;
    esp_err_t err = gpio_set_level(PRG32_PIN_LCD_BL, on ? level : !level);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LCD backlight GPIO failed: %s", esp_err_to_name(err));
    }
#else
    (void)on;
#endif
}

static void lcd_reset(void) {
#if PRG32_PIN_LCD_RST >= 0
    esp_err_t err = gpio_set_level(PRG32_PIN_LCD_RST, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LCD reset low failed: %s", esp_err_to_name(err));
        return;
    }
    esp_rom_delay_us(20000);
    err = gpio_set_level(PRG32_PIN_LCD_RST, 1);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LCD reset high failed: %s", esp_err_to_name(err));
        return;
    }
    esp_rom_delay_us(120000);
#endif
}

static void lcd_init_sequence(void) {
    static const uint8_t cmd_ef[] = {0x03, 0x80, 0x02};
    static const uint8_t cmd_cf[] = {0x00, 0xc1, 0x30};
    static const uint8_t cmd_ed[] = {0x64, 0x03, 0x12, 0x81};
    static const uint8_t cmd_e8[] = {0x85, 0x00, 0x78};
    static const uint8_t cmd_cb[] = {0x39, 0x2c, 0x00, 0x34, 0x02};
    static const uint8_t cmd_f7[] = {0x20};
    static const uint8_t cmd_ea[] = {0x00, 0x00};
    static const uint8_t power1[] = {0x23};
    static const uint8_t power2[] = {0x10};
    static const uint8_t vcom1[] = {0x3e, 0x28};
    static const uint8_t vcom2[] = {0x86};
    static const uint8_t madctl[] = {PRG32_LCD_MADCTL};
    static const uint8_t pixfmt[] = {0x55};
    static const uint8_t frame_rate[] = {0x00, 0x18};
    static const uint8_t display_fn[] = {0x08, 0x82, 0x27};
    static const uint8_t gamma_fn[] = {0x00};
    static const uint8_t gamma_set[] = {0x01};
    static const uint8_t gamma_pos[] = {
        0x0f, 0x31, 0x2b, 0x0c, 0x0e, 0x08, 0x4e, 0xf1,
        0x37, 0x07, 0x10, 0x03, 0x0e, 0x09, 0x00,
    };
    static const uint8_t gamma_neg[] = {
        0x00, 0x0e, 0x14, 0x03, 0x11, 0x07, 0x31, 0xc1,
        0x48, 0x08, 0x0f, 0x0c, 0x31, 0x36, 0x0f,
    };

    lcd_cmd(0x01);
    esp_rom_delay_us(150000);

    lcd_cmd_data(0xef, cmd_ef, sizeof(cmd_ef));
    lcd_cmd_data(0xcf, cmd_cf, sizeof(cmd_cf));
    lcd_cmd_data(0xed, cmd_ed, sizeof(cmd_ed));
    lcd_cmd_data(0xe8, cmd_e8, sizeof(cmd_e8));
    lcd_cmd_data(0xcb, cmd_cb, sizeof(cmd_cb));
    lcd_cmd_data(0xf7, cmd_f7, sizeof(cmd_f7));
    lcd_cmd_data(0xea, cmd_ea, sizeof(cmd_ea));
    lcd_cmd_data(0xc0, power1, sizeof(power1));
    lcd_cmd_data(0xc1, power2, sizeof(power2));
    lcd_cmd_data(0xc5, vcom1, sizeof(vcom1));
    lcd_cmd_data(0xc7, vcom2, sizeof(vcom2));
    lcd_cmd_data(0x36, madctl, sizeof(madctl));
    lcd_cmd_data(0x3a, pixfmt, sizeof(pixfmt));
    lcd_cmd_data(0xb1, frame_rate, sizeof(frame_rate));
    lcd_cmd_data(0xb6, display_fn, sizeof(display_fn));
    lcd_cmd_data(0xf2, gamma_fn, sizeof(gamma_fn));
    lcd_cmd_data(0x26, gamma_set, sizeof(gamma_set));
    lcd_cmd_data(0xe0, gamma_pos, sizeof(gamma_pos));
    lcd_cmd_data(0xe1, gamma_neg, sizeof(gamma_neg));

    lcd_cmd(0x11);
    esp_rom_delay_us(120000);
    lcd_cmd(0x13);
    lcd_cmd(0x20);
    lcd_cmd(0x29);
    esp_rom_delay_us(20000);
}

static void lcd_log_identity(void) {
    uint8_t id[4] = {0};
    uint8_t status[5] = {0};
    if (lcd_read_command(0x04, id, sizeof(id))) {
        ESP_LOGI(TAG,
                 "ILI9341 read ID: %02x %02x %02x %02x",
                 id[0],
                 id[1],
                 id[2],
                 id[3]);
    }
    if (lcd_read_command(0x09, status, sizeof(status))) {
        ESP_LOGI(TAG,
                 "ILI9341 read status: %02x %02x %02x %02x %02x",
                 status[0],
                 status[1],
                 status[2],
                 status[3],
                 status[4]);
    }
}

static void lcd_addr_raw(int x0, int y0, int x1, int y1) {
    uint8_t d[4];
    d[0] = (uint8_t)(x0 >> 8);
    d[1] = (uint8_t)x0;
    d[2] = (uint8_t)(x1 >> 8);
    d[3] = (uint8_t)x1;
    lcd_cmd_data(0x2a, d, sizeof(d));
    d[0] = (uint8_t)(y0 >> 8);
    d[1] = (uint8_t)y0;
    d[2] = (uint8_t)(y1 >> 8);
    d[3] = (uint8_t)y1;
    lcd_cmd_data(0x2b, d, sizeof(d));
    lcd_cmd(0x2c);
}

static void lcd_addr(int x0, int y0, int x1, int y1) {
    lcd_addr_raw(x0, y0 + 20, x1, y1 + 20);
}

#if PRG32_LCD_BOOT_TEST_MS > 0
static void lcd_fill_solid_raw(int x0,
                               int y0,
                               int x1,
                               int y1,
                               uint16_t color) {
    if (!g_lcd_ready || x1 < x0 || y1 < y0) {
        return;
    }
    int width = x1 - x0 + 1;
    uint16_t wire = rgb565_wire(color);
    for (int i = 0; i < width; ++i) {
        g_line_buf[i] = wire;
    }
    lcd_addr_raw(x0, y0, x1, y1);
    for (int y = y0; y <= y1; ++y) {
        lcd_data(g_line_buf, width * (int)sizeof(g_line_buf[0]));
#if PRG32_LCD_SOFT_SPI
        if ((y & 7) == 7) {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
#endif
    }
}
#endif

static void lcd_boot_test_pattern(void) {
#if PRG32_LCD_BOOT_TEST_MS > 0
#if !PRG32_LCD_SOFT_SPI
    static const uint16_t colors[] = {
        PRG32_COLOR_RED,
        PRG32_COLOR_GREEN,
        PRG32_COLOR_BLUE,
        PRG32_COLOR_WHITE,
    };
#endif
    ESP_LOGI(TAG, "drawing LCD boot test pattern");
#if PRG32_LCD_SOFT_SPI
    lcd_fill_solid_raw(0, 0, 63, 63, PRG32_COLOR_RED);
    lcd_fill_solid_raw(256, 0, 319, 63, PRG32_COLOR_GREEN);
    lcd_fill_solid_raw(0, 176, 63, 239, PRG32_COLOR_BLUE);
    lcd_fill_solid_raw(256, 176, 319, 239, PRG32_COLOR_WHITE);
    lcd_fill_solid_raw(128, 88, 191, 151, PRG32_COLOR_YELLOW);
#else
    int x = 0;
    for (int i = 0; i < 4; ++i) {
        int x1 = (i == 3) ? 319 : x + 79;
        lcd_fill_solid_raw(x, 0, x1, 239, colors[i]);
        x = x1 + 1;
    }
#endif
    esp_rom_delay_us(PRG32_LCD_BOOT_TEST_MS * 1000u);
#if PRG32_PIN_LCD_BL >= 0
    ESP_LOGI(TAG, "LCD boot test: forcing BL high");
    gpio_set_level(PRG32_PIN_LCD_BL, 1);
    esp_rom_delay_us(PRG32_LCD_BOOT_TEST_MS * 1000u);
    ESP_LOGI(TAG, "LCD boot test: forcing BL low");
    gpio_set_level(PRG32_PIN_LCD_BL, 0);
    esp_rom_delay_us(PRG32_LCD_BOOT_TEST_MS * 1000u);
    lcd_backlight(1);
#endif
#endif
}

void prg32_display_init(void) {
#if !PRG32_LCD_SOFT_SPI
    spi_bus_config_t bus = {
        .mosi_io_num = PRG32_PIN_LCD_MOSI,
        .miso_io_num = PRG32_PIN_LCD_MISO,
        .sclk_io_num = PRG32_PIN_LCD_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = PRG32_GAME_W * 2
    };
    spi_device_interface_config_t dev = {
        .clock_speed_hz = PRG32_LCD_SPI_CLOCK_HZ,
        .mode = 0,
        .spics_io_num = -1,
        .queue_size = 7,
        .flags = SPI_DEVICE_HALFDUPLEX
    };
#endif
    ESP_LOGI(TAG,
#if PRG32_LCD_SOFT_SPI
             "ILI9341 software SPI MOSI=%d MISO=%d SCLK=%d CS=%d DC=%d RST=%d BL=%d",
#else
             "ILI9341 SPI2 MOSI=%d MISO=%d SCLK=%d CS=%d DC=%d RST=%d BL=%d clock=%d",
#endif
             PRG32_PIN_LCD_MOSI,
             PRG32_PIN_LCD_MISO,
             PRG32_PIN_LCD_SCLK,
             PRG32_PIN_LCD_CS,
             PRG32_PIN_LCD_DC,
             PRG32_PIN_LCD_RST,
             PRG32_PIN_LCD_BL
#if !PRG32_LCD_SOFT_SPI
             ,
             PRG32_LCD_SPI_CLOCK_HZ);
#else
             );
#endif
    esp_err_t err = lcd_prepare_output_pin(PRG32_PIN_LCD_DC, "DC");
    if (err != ESP_OK) {
        return;
    }
    err = lcd_prepare_output_pin(PRG32_PIN_LCD_CS, "CS");
    if (err != ESP_OK) {
        return;
    }
    lcd_deselect();
    err = lcd_prepare_output_pin(PRG32_PIN_LCD_RST, "RST");
    if (err != ESP_OK) {
        return;
    }
    err = lcd_prepare_output_pin(PRG32_PIN_LCD_BL, "BL");
    if (err != ESP_OK) {
        return;
    }
#if !PRG32_LCD_SOFT_SPI
    err = lcd_prepare_output_pin(PRG32_PIN_LCD_MOSI, "MOSI");
    if (err != ESP_OK) {
        return;
    }
    err = lcd_prepare_output_pin(PRG32_PIN_LCD_SCLK, "SCLK");
    if (err != ESP_OK) {
        return;
    }
    err = lcd_prepare_input_pin(PRG32_PIN_LCD_MISO, "MISO");
    if (err != ESP_OK) {
        return;
    }
#endif
    lcd_backlight(0);
#if PRG32_LCD_SOFT_SPI
    err = lcd_soft_spi_init();
    if (err != ESP_OK) {
        lcd_backlight(1);
        return;
    }
#else
    err = spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_CH_AUTO);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LCD SPI bus init failed: %s", esp_err_to_name(err));
        lcd_backlight(1);
        return;
    }
    err = spi_bus_add_device(SPI2_HOST, &dev, &g_lcd);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LCD SPI device add failed: %s", esp_err_to_name(err));
        lcd_backlight(1);
        return;
    }
#endif
    lcd_reset();
    lcd_init_sequence();
    lcd_log_identity();
    lcd_backlight(1);
    g_lcd_ready = 1;
    ESP_LOGI(TAG, "ILI9341 initialization complete");
    lcd_boot_test_pattern();
    dirty_reset();
}

uint32_t prg32_ticks_ms(void) {
    return (uint32_t)(esp_timer_get_time() / 1000u);
}

void prg32_gfx_clear(uint16_t color) {
    for (int i = 0; i < PRG32_GAME_W * PRG32_GAME_H; ++i) {
        g_fb[i] = color;
    }
    dirty_add(0, 0, PRG32_GAME_W, PRG32_GAME_H);
}

void prg32_gfx_pixel(int x, int y, uint16_t color) {
    if ((unsigned)x >= PRG32_GAME_W || (unsigned)y >= PRG32_GAME_H) {
        return;
    }
    g_fb[y * PRG32_GAME_W + x] = color;
    dirty_add(x, y, 1, 1);
}

void prg32_gfx_rect(int x, int y, int w, int h, uint16_t color) {
    if (w <= 0 || h <= 0) {
        return;
    }
    int x0 = x;
    int y0 = y;
    int x1 = x + w;
    int y1 = y + h;
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > PRG32_GAME_W) x1 = PRG32_GAME_W;
    if (y1 > PRG32_GAME_H) y1 = PRG32_GAME_H;
    if (x0 >= x1 || y0 >= y1) {
        return;
    }
    for (int py = y0; py < y1; ++py) {
        for (int px = x0; px < x1; ++px) {
            g_fb[py * PRG32_GAME_W + px] = color;
        }
    }
    dirty_add(x0, y0, x1 - x0, y1 - y0);
}

void prg32_gfx_text8(int x, int y, const char *s, uint16_t fg, uint16_t bg) {
    if (!s) {
        return;
    }
    while (*s) {
        const uint8_t *glyph = font_glyph((unsigned char)*s++);
        for (int row = 0; row < 8; ++row) {
            uint8_t bits = glyph[row];
            int py = y + row;
            if ((unsigned)py >= PRG32_GAME_H) {
                continue;
            }
            for (int col = 0; col < 8; ++col) {
                int px = x + col;
                if ((unsigned)px >= PRG32_GAME_W) {
                    continue;
                }
                g_fb[py * PRG32_GAME_W + px] =
                    (bits & (1u << (7 - col))) ? fg : bg;
            }
        }
        dirty_add(x, y, 8, 8);
        x += 8;
    }
}

void prg32_gfx_present(void) {
    if (!g_lcd_ready) {
        return;
    }
    if (g_dirty_x1 < g_dirty_x0) {
        return;
    }
    int x0 = g_dirty_x0;
    int y0 = g_dirty_y0;
    int x1 = g_dirty_x1;
    int y1 = g_dirty_y1;
    int width = x1 - x0 + 1;
    lcd_addr(x0, y0, x1, y1);
    for (int y = y0; y <= y1; ++y) {
        for (int x = 0; x < width; ++x) {
            g_line_buf[x] = rgb565_wire(g_fb[y * PRG32_GAME_W + x0 + x]);
        }
        lcd_data(g_line_buf, width * (int)sizeof(g_line_buf[0]));
#if PRG32_LCD_SOFT_SPI
        if ((y & 7) == 7) {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
#endif
    }
    dirty_reset();
}
