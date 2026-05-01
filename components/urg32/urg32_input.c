#include "urg32.h"
#include "urg32_config.h"
#include "driver/gpio.h"

static void pin_in(int p) {
    if (p < 0) {
        return;
    }
    gpio_set_direction(p, GPIO_MODE_INPUT);
    gpio_set_pull_mode(p, GPIO_PULLUP_ONLY);
}

void urg32_controller_bridge_init(void);

void urg32_input_init(void) {
    pin_in(URG32_PIN_BTN_LEFT); pin_in(URG32_PIN_BTN_RIGHT); pin_in(URG32_PIN_BTN_UP);
    pin_in(URG32_PIN_BTN_DOWN); pin_in(URG32_PIN_BTN_A); pin_in(URG32_PIN_BTN_B);
    urg32_controller_bridge_init();
}

uint32_t urg32_input_read(void) {
    return urg32_controller_read();
}
