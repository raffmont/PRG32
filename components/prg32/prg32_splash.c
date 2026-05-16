#include "prg32.h"
#include "prg32_config.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stddef.h>

#ifndef CONFIG_PRG32_SPLASH_ENABLED
#define CONFIG_PRG32_SPLASH_ENABLED 1
#endif

#ifndef CONFIG_PRG32_SPLASH_DURATION_MS
#define CONFIG_PRG32_SPLASH_DURATION_MS 900
#endif

#ifndef CONFIG_PRG32_SPLASH_SOUND_ENABLED
#define CONFIG_PRG32_SPLASH_SOUND_ENABLED 1
#endif

#ifndef CONFIG_PRG32_AUDIO_ENABLED
#define CONFIG_PRG32_AUDIO_ENABLED 0
#endif

#ifndef CONFIG_PRG32_DISPLAY_QEMU_RGB
#define CONFIG_PRG32_DISPLAY_QEMU_RGB 0
#endif

#ifndef CONFIG_PRG32_AUDIO_I2S_BCLK_GPIO
#define CONFIG_PRG32_AUDIO_I2S_BCLK_GPIO -1
#endif

#ifndef CONFIG_PRG32_AUDIO_I2S_LRCLK_GPIO
#define CONFIG_PRG32_AUDIO_I2S_LRCLK_GPIO -1
#endif

#ifndef CONFIG_PRG32_AUDIO_I2S_DATA_GPIO
#define CONFIG_PRG32_AUDIO_I2S_DATA_GPIO -1
#endif

#ifndef CONFIG_PRG32_AUDIO_I2S_SD_GPIO
#define CONFIG_PRG32_AUDIO_I2S_SD_GPIO -1
#endif

#define PRG32_SPLASH_LOGO_W 320
#define PRG32_SPLASH_LOGO_H 200
#define PRG32_SPLASH_WELCOME_SAMPLE_ID 63
#define PRG32_SPLASH_WELCOME_INSTRUMENT_ID 31

extern const uint16_t prg32_splash_logo[];

static const uint8_t welcome_wave[] = {
    128, 166, 202, 231, 250, 255, 246, 224,
    192, 154, 114, 76, 44, 20, 6, 0,
    6, 20, 44, 76, 114, 154, 192, 224,
    246, 255, 250, 231, 202, 166,
};

static int text_center_x(const char *text) {
    size_t len = 0;
    if (text) {
        while (text[len] != '\0') {
            len++;
        }
    }
    int width = (int)len * 8;
    if (width >= PRG32_GAME_W) {
        return 0;
    }
    return (PRG32_GAME_W - width) / 2;
}

static void prg32_splash_draw_logo(void) {
    for (int y = 0; y < PRG32_SPLASH_LOGO_H; ++y) {
        for (int x = 0; x < PRG32_SPLASH_LOGO_W; ++x) {
            uint16_t color = prg32_splash_logo[y * PRG32_SPLASH_LOGO_W + x];
            prg32_gfx_pixel(x, y, color);
        }
    }
}

static int pin_matches(int pin, const int *reserved, size_t count) {
    if (pin < 0) {
        return 0;
    }
    for (size_t i = 0; i < count; ++i) {
        if (pin == reserved[i]) {
            return 1;
        }
    }
    return 0;
}

static int splash_i2s_pins_safe(void) {
    const int reserved[] = {
        PRG32_PIN_LCD_MOSI,
        PRG32_PIN_LCD_MISO,
        PRG32_PIN_LCD_SCLK,
        PRG32_PIN_LCD_CS,
        PRG32_PIN_LCD_DC,
        PRG32_PIN_LCD_RST,
        PRG32_PIN_LCD_BL,
        PRG32_PIN_BTN_LEFT,
        PRG32_PIN_BTN_RIGHT,
        PRG32_PIN_BTN_UP,
        PRG32_PIN_BTN_DOWN,
        PRG32_PIN_BTN_A,
        PRG32_PIN_BTN_B,
        PRG32_PIN_BTN_START,
        PRG32_PIN_SETUP,
        PRG32_PIN_P2_LEFT,
        PRG32_PIN_P2_RIGHT,
        PRG32_PIN_P2_UP,
        PRG32_PIN_P2_DOWN,
        PRG32_PIN_P2_A,
        PRG32_PIN_P2_B,
        PRG32_PIN_P2_START,
    };

    if (CONFIG_PRG32_DISPLAY_QEMU_RGB) {
        return 0;
    }
    return !pin_matches(CONFIG_PRG32_AUDIO_I2S_BCLK_GPIO,
                        reserved,
                        sizeof(reserved) / sizeof(reserved[0])) &&
           !pin_matches(CONFIG_PRG32_AUDIO_I2S_LRCLK_GPIO,
                        reserved,
                        sizeof(reserved) / sizeof(reserved[0])) &&
           !pin_matches(CONFIG_PRG32_AUDIO_I2S_DATA_GPIO,
                        reserved,
                        sizeof(reserved) / sizeof(reserved[0])) &&
           !pin_matches(CONFIG_PRG32_AUDIO_I2S_SD_GPIO,
                        reserved,
                        sizeof(reserved) / sizeof(reserved[0]));
}

static int prg32_splash_prepare_sound(void) {
#if CONFIG_PRG32_AUDIO_ENABLED && CONFIG_PRG32_SPLASH_SOUND_ENABLED
    if (!prg32_audio_is_ready() &&
        (!splash_i2s_pins_safe() || !prg32_audio_init(NULL))) {
        return 0;
    }

    if (prg32_audio_register_sample(PRG32_SPLASH_WELCOME_SAMPLE_ID,
                                    welcome_wave,
                                    sizeof(welcome_wave),
                                    60,
                                    PRG32_AUDIO_SAMPLE_LOOP,
                                    0,
                                    sizeof(welcome_wave)) != 0) {
        return 0;
    }

    prg32_instrument_desc_t instrument = {
        .sample_id = PRG32_SPLASH_WELCOME_SAMPLE_ID,
        .default_volume = 120,
        .default_pan = PRG32_AUDIO_PAN_CENTER,
        .sustain = 255,
    };
    return prg32_audio_register_instrument(PRG32_SPLASH_WELCOME_INSTRUMENT_ID,
                                           &instrument) == 0;
#else
    return 0;
#endif
}

static void prg32_splash_play_wait(uint32_t duration_ms) {
    if (duration_ms > 0) {
        vTaskDelay(pdMS_TO_TICKS(duration_ms));
    }
}

static void prg32_splash_play_pwm_welcome(uint32_t duration_ms) {
#if PRG32_PIN_BUZZER >= 0
    const uint32_t step_ms = duration_ms >= 360 ? 120 : duration_ms / 3;
    const uint32_t tail_ms =
        duration_ms > step_ms * 2 ? duration_ms - step_ms * 2 : step_ms;
    prg32_audio_note(72, step_ms);
    prg32_audio_note(76, step_ms);
    prg32_audio_note(79, tail_ms);
#else
    prg32_splash_play_wait(duration_ms);
#endif
}

static void prg32_splash_play_i2s_welcome(uint32_t duration_ms) {
    if (!prg32_splash_prepare_sound()) {
        prg32_splash_play_pwm_welcome(duration_ms);
        return;
    }

    const uint32_t step_ms = duration_ms >= 360 ? 120 : duration_ms / 3;
    const uint32_t tail_ms =
        duration_ms > step_ms * 2 ? duration_ms - step_ms * 2 : step_ms;
    prg32_audio_note_on(0,
                        PRG32_SPLASH_WELCOME_INSTRUMENT_ID,
                        72,
                        120);
    vTaskDelay(pdMS_TO_TICKS(step_ms));
    prg32_audio_note_on(0,
                        PRG32_SPLASH_WELCOME_INSTRUMENT_ID,
                        76,
                        120);
    vTaskDelay(pdMS_TO_TICKS(step_ms));
    prg32_audio_note_on(0,
                        PRG32_SPLASH_WELCOME_INSTRUMENT_ID,
                        79,
                        120);
    vTaskDelay(pdMS_TO_TICKS(tail_ms));
    prg32_audio_note_off(0);
}

static void prg32_splash_play_welcome(uint32_t duration_ms) {
#if CONFIG_PRG32_SPLASH_SOUND_ENABLED
    prg32_splash_play_i2s_welcome(duration_ms);
#else
    prg32_splash_play_wait(duration_ms);
#endif
}

void prg32_splash_draw(const char *title,
                       const char *subtitle,
                       uint16_t bg,
                       uint16_t fg,
                       uint16_t accent) {
    if (!title) {
        title = "PRG32";
    }
    if (!subtitle) {
        subtitle = "";
    }

    prg32_gfx_clear(bg);
    prg32_gfx_rect(0, 0, PRG32_GAME_W, 6, accent);
    prg32_gfx_rect(0, PRG32_GAME_H - 6, PRG32_GAME_W, 6, accent);
    prg32_gfx_rect(34, 54, 252, 2, accent);
    prg32_gfx_rect(34, 138, 252, 2, accent);

    for (int i = 0; i < 5; ++i) {
        int x = 72 + i * 36;
        prg32_gfx_rect(x, 72, 20, 20, accent);
        prg32_gfx_rect(x + 4, 76, 12, 12, bg);
    }

    int title_x = text_center_x(title);
    int subtitle_x = text_center_x(subtitle);
    prg32_gfx_text8(title_x + 1, 98 + 1, title, accent, bg);
    prg32_gfx_text8(title_x, 98, title, fg, bg);
    prg32_gfx_text8(subtitle_x, 116, subtitle, fg, bg);
}

void prg32_splash_show(const char *title,
                       const char *subtitle,
                       uint32_t duration_ms,
                       uint16_t bg,
                       uint16_t fg,
                       uint16_t accent) {
    prg32_splash_draw(title, subtitle, bg, fg, accent);
    prg32_gfx_present();
    if (duration_ms > 0) {
        vTaskDelay(pdMS_TO_TICKS(duration_ms));
    }
}

void prg32_splash_show_default(void) {
#if CONFIG_PRG32_SPLASH_ENABLED
    prg32_splash_draw_logo();
    prg32_gfx_present();
    prg32_splash_play_welcome(CONFIG_PRG32_SPLASH_DURATION_MS);
#endif
}
