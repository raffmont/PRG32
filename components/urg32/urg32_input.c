#include "urg32.h"
#include "urg32_config.h"
#include "driver/gpio.h"

static void pin_in(int p) { gpio_set_direction(p, GPIO_MODE_INPUT); gpio_set_pull_mode(p, GPIO_PULLUP_ONLY); }

void urg32_input_init(void) {
    pin_in(URG32_PIN_BTN_LEFT); pin_in(URG32_PIN_BTN_RIGHT); pin_in(URG32_PIN_BTN_UP);
    pin_in(URG32_PIN_BTN_DOWN); pin_in(URG32_PIN_BTN_A); pin_in(URG32_PIN_BTN_B);
}

uint32_t urg32_input_read(void) {
    uint32_t v = 0;
    if (!gpio_get_level(URG32_PIN_BTN_LEFT)) v |= URG32_BTN_LEFT;
    if (!gpio_get_level(URG32_PIN_BTN_RIGHT)) v |= URG32_BTN_RIGHT;
    if (!gpio_get_level(URG32_PIN_BTN_UP)) v |= URG32_BTN_UP;
    if (!gpio_get_level(URG32_PIN_BTN_DOWN)) v |= URG32_BTN_DOWN;
    if (!gpio_get_level(URG32_PIN_BTN_A)) v |= URG32_BTN_A;
    if (!gpio_get_level(URG32_PIN_BTN_B)) v |= URG32_BTN_B;
    return v;
}
