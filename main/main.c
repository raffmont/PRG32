#include "prg32.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void) {
    prg32_init();
    prg32_console_clear();
    prg32_console_write("PRG32 Hello World\n");

    uint32_t cart_generation = 0;
    while (1) {
        if (prg32_cart_is_loaded()) {
            uint32_t current_generation = prg32_cart_generation();
            if (current_generation != cart_generation) {
                cart_generation = current_generation;
                prg32_console_clear();
                prg32_cart_call_init();
            }
            prg32_cart_call_update();
            prg32_cart_call_draw();
            prg32_gfx_present();
            vTaskDelay(pdMS_TO_TICKS(33));
        } else {
            prg32_gfx_present();
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}
