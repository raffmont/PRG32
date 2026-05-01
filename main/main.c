#include "urg32.h"
#include "urg32_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void urg32_display_init(void);
void urg32_input_init(void);
void urg32_audio_init(void);

void pong_ascii_init(void);
void pong_ascii_update(void);
void pong_ascii_draw(void);
void pong_graphics_init(void);
void pong_graphics_update(void);
void pong_graphics_draw(void);
void breakout_ascii_init(void);
void breakout_ascii_update(void);
void breakout_ascii_draw(void);
void breakout_graphics_init(void);
void breakout_graphics_update(void);
void breakout_graphics_draw(void);
void invaders_ascii_init(void);
void invaders_ascii_update(void);
void invaders_ascii_draw(void);
void invaders_graphics_init(void);
void invaders_graphics_update(void);
void invaders_graphics_draw(void);
void pacman_ascii_init(void);
void pacman_ascii_update(void);
void pacman_ascii_draw(void);
void pacman_graphics_init(void);
void pacman_graphics_update(void);
void pacman_graphics_draw(void);

#if URG32_SELECTED_GAME == URG32_GAME_PONG_ASCII
#define SELECTED_INIT pong_ascii_init
#define SELECTED_UPDATE pong_ascii_update
#define SELECTED_DRAW pong_ascii_draw
#elif URG32_SELECTED_GAME == URG32_GAME_PONG_GRAPHICS
#define SELECTED_INIT pong_graphics_init
#define SELECTED_UPDATE pong_graphics_update
#define SELECTED_DRAW pong_graphics_draw
#elif URG32_SELECTED_GAME == URG32_GAME_BREAKOUT_ASCII
#define SELECTED_INIT breakout_ascii_init
#define SELECTED_UPDATE breakout_ascii_update
#define SELECTED_DRAW breakout_ascii_draw
#elif URG32_SELECTED_GAME == URG32_GAME_BREAKOUT_GRAPHICS
#define SELECTED_INIT breakout_graphics_init
#define SELECTED_UPDATE breakout_graphics_update
#define SELECTED_DRAW breakout_graphics_draw
#elif URG32_SELECTED_GAME == URG32_GAME_INVADERS_ASCII
#define SELECTED_INIT invaders_ascii_init
#define SELECTED_UPDATE invaders_ascii_update
#define SELECTED_DRAW invaders_ascii_draw
#elif URG32_SELECTED_GAME == URG32_GAME_INVADERS_GRAPHICS
#define SELECTED_INIT invaders_graphics_init
#define SELECTED_UPDATE invaders_graphics_update
#define SELECTED_DRAW invaders_graphics_draw
#elif URG32_SELECTED_GAME == URG32_GAME_PACMAN_ASCII
#define SELECTED_INIT pacman_ascii_init
#define SELECTED_UPDATE pacman_ascii_update
#define SELECTED_DRAW pacman_ascii_draw
#elif URG32_SELECTED_GAME == URG32_GAME_PACMAN_GRAPHICS
#define SELECTED_INIT pacman_graphics_init
#define SELECTED_UPDATE pacman_graphics_update
#define SELECTED_DRAW pacman_graphics_draw
#else
#error "Unsupported URG32_SELECTED_GAME value"
#endif

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
    SELECTED_INIT();
}

static void selected_update(void) {
    SELECTED_UPDATE();
}

static void selected_draw(void) {
    SELECTED_DRAW();
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
