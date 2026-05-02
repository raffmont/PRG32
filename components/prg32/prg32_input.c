#include "prg32.h"
#include "prg32_config.h"
#include "driver/gpio.h"

static void pin_in(int p) {
    if (p < 0) {
        return;
    }
    gpio_set_direction(p, GPIO_MODE_INPUT);
    gpio_set_pull_mode(p, GPIO_PULLUP_ONLY);
}

void prg32_controller_bridge_init(void);

void prg32_input_init(void) {
    pin_in(PRG32_PIN_BTN_LEFT); pin_in(PRG32_PIN_BTN_RIGHT); pin_in(PRG32_PIN_BTN_UP);
    pin_in(PRG32_PIN_BTN_DOWN); pin_in(PRG32_PIN_BTN_A); pin_in(PRG32_PIN_BTN_B);
    pin_in(PRG32_PIN_BTN_START);
    prg32_controller_bridge_init();
}

uint32_t prg32_input_read(void) {
    return prg32_controller_read();
}
