#include "prg32.h"
#include "prg32_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void) {
    prg32_init();
    prg32_console_clear();
    prg32_console_write("PRG32 Hello World\n");
#if PRG32_DEBUG
    prg32_console_write("PRG32 DEBUG enabled\n");
#endif

    uint32_t cart_generation = 0;
    while (1) {
        uint32_t input_snapshot = prg32_controller_read();
        prg32_diag_set_input_state(input_snapshot);

        if (prg32_cart_is_loaded()) {
            uint32_t current_generation = prg32_cart_generation();
            if (current_generation != cart_generation) {
                cart_generation = current_generation;
                prg32_console_clear();
                prg32_cart_call_init();
            }
            prg32_cart_call_update();
            prg32_cart_call_draw();
#if PRG32_DEBUG
            prg32_debug_overlay_draw(1, 0, 0, input_snapshot, prg32_diag_frame_count());
#endif
            prg32_gfx_present();
            prg32_diag_increment_frame();
            vTaskDelay(pdMS_TO_TICKS(33));
        } else {
            prg32_gfx_present();
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}
