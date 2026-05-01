#include "prg32.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void) {
    prg32_init();
    prg32_console_clear();
    prg32_console_write("PRG32 Hello World\n");

    while (1) {
        prg32_gfx_present();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
