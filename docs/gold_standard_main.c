/*
 * PRG32 Gold Standard Example: main.c
 *
 * This version is refactored for clarity and teaching purposes.
 * Behavior is identical to the original runtime loop.
 */

#include "prg32.h"
#include "prg32_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static void handle_loaded_cartridge(uint32_t input_snapshot,
                                    uint32_t *cart_generation) {

    uint32_t current_generation = prg32_cart_generation();

    /* Detect cartridge change */
    if (current_generation != *cart_generation) {
        *cart_generation = current_generation;
        prg32_console_clear();
        prg32_cart_call_init();
    }

    /* Standard game loop */
    prg32_cart_call_update();
    prg32_cart_call_draw();

#if PRG32_DEBUG
    prg32_debug_overlay_draw(1,
                             0,
                             0,
                             input_snapshot,
                             prg32_diag_frame_count());
#endif

    prg32_gfx_present();
    prg32_diag_increment_frame();

    /* ~30 FPS */
    vTaskDelay(pdMS_TO_TICKS(33));
}

static void handle_no_cartridge(void) {
    prg32_gfx_present();

    /* Idle delay */
    vTaskDelay(pdMS_TO_TICKS(1000));
}

void app_main(void) {

    prg32_init();

    prg32_console_clear();
    prg32_console_write("PRG32 Hello World\n");

#if PRG32_DEBUG
    prg32_console_write("PRG32 DEBUG enabled\n");
#endif

    uint32_t cart_generation = 0;

    while (1) {

        /* Read input once per frame */
        uint32_t input_snapshot = prg32_controller_read();
        prg32_diag_set_input_state(input_snapshot);

        if (prg32_cart_is_loaded()) {
            handle_loaded_cartridge(input_snapshot, &cart_generation);
        } else {
            handle_no_cartridge();
        }
    }
}
