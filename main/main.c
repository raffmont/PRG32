#include "urg32.h"
#include "urg32_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void urg32_display_init(void);
void urg32_input_init(void);
void urg32_audio_init(void);

void pong_ascii_init(void); void pong_ascii_update(void); void pong_ascii_draw(void);
void pong_graphics_init(void); void pong_graphics_update(void); void pong_graphics_draw(void);
void breakout_ascii_init(void); void breakout_ascii_update(void); void breakout_ascii_draw(void);
void breakout_graphics_init(void); void breakout_graphics_update(void); void breakout_graphics_draw(void);
void invaders_ascii_init(void); void invaders_ascii_update(void); void invaders_ascii_draw(void);
void invaders_graphics_init(void); void invaders_graphics_update(void); void invaders_graphics_draw(void);
void pacman_ascii_init(void); void pacman_ascii_update(void); void pacman_ascii_draw(void);
void pacman_graphics_init(void); void pacman_graphics_update(void); void pacman_graphics_draw(void);

void urg32_init(void) {
    urg32_display_init();
    urg32_input_init();
    urg32_audio_init();
    urg32_wifi_scores_init();
    urg32_scores_api_start();
    urg32_set_mode(URG32_DEFAULT_MODE);
    urg32_console_clear();
}

static void selected_init(void) {
#if URG32_SELECTED_GAME == URG32_GAME_PONG_ASCII
    pong_ascii_init();
#elif URG32_SELECTED_GAME == URG32_GAME_PONG_GRAPHICS
    pong_graphics_init();
#elif URG32_SELECTED_GAME == URG32_GAME_BREAKOUT_ASCII
    breakout_ascii_init();
#elif URG32_SELECTED_GAME == URG32_GAME_BREAKOUT_GRAPHICS
    breakout_graphics_init();
#elif URG32_SELECTED_GAME == URG32_GAME_INVADERS_ASCII
    invaders_ascii_init();
#elif URG32_SELECTED_GAME == URG32_GAME_INVADERS_GRAPHICS
    invaders_graphics_init();
#elif URG32_SELECTED_GAME == URG32_GAME_PACMAN_ASCII
    pacman_ascii_init();
#else
    pacman_graphics_init();
#endif
}

static void selected_update(void) {
#if URG32_SELECTED_GAME == URG32_GAME_PONG_ASCII
    pong_ascii_update();
#elif URG32_SELECTED_GAME == URG32_GAME_PONG_GRAPHICS
    pong_graphics_update();
#elif URG32_SELECTED_GAME == URG32_GAME_BREAKOUT_ASCII
    breakout_ascii_update();
#elif URG32_SELECTED_GAME == URG32_GAME_BREAKOUT_GRAPHICS
    breakout_graphics_update();
#elif URG32_SELECTED_GAME == URG32_GAME_INVADERS_ASCII
    invaders_ascii_update();
#elif URG32_SELECTED_GAME == URG32_GAME_INVADERS_GRAPHICS
    invaders_graphics_update();
#elif URG32_SELECTED_GAME == URG32_GAME_PACMAN_ASCII
    pacman_ascii_update();
#else
    pacman_graphics_update();
#endif
}

static void selected_draw(void) {
#if URG32_SELECTED_GAME == URG32_GAME_PONG_ASCII
    pong_ascii_draw();
#elif URG32_SELECTED_GAME == URG32_GAME_PONG_GRAPHICS
    pong_graphics_draw();
#elif URG32_SELECTED_GAME == URG32_GAME_BREAKOUT_ASCII
    breakout_ascii_draw();
#elif URG32_SELECTED_GAME == URG32_GAME_BREAKOUT_GRAPHICS
    breakout_graphics_draw();
#elif URG32_SELECTED_GAME == URG32_GAME_INVADERS_ASCII
    invaders_ascii_draw();
#elif URG32_SELECTED_GAME == URG32_GAME_INVADERS_GRAPHICS
    invaders_graphics_draw();
#elif URG32_SELECTED_GAME == URG32_GAME_PACMAN_ASCII
    pacman_ascii_draw();
#else
    pacman_graphics_draw();
#endif
}

void app_main(void) {
    urg32_init();
    selected_init();
    while (1) {
        selected_update();
        selected_draw();
        urg32_gfx_present();
        vTaskDelay(pdMS_TO_TICKS(16));
    }
}
