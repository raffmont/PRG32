#include "urg32.h"
#include "urg32_config.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include <string.h>

/*
 * URG32 controller layer.
 *
 * ESP32-C6 has a USB Serial/JTAG device controller, but it is not a normal
 * USB host port for plugging in arbitrary USB HID gamepads. Therefore this
 * file supports three classroom-friendly input backends:
 *   1. GPIO buttons on the reference breadboard/PCB.
 *   2. A USB-HID-host bridge, e.g. RP2040/CH559/ESP32-S3, connected by UART.
 *   3. A host-terminal keyboard/debug mode through the same UART protocol.
 *
 * The game sees only the stable URG32 bitmask: LEFT/RIGHT/UP/DOWN/A/B/START.
 * This is intentionally similar to memory-mapped input registers on 1980s
 * consoles, which makes it a useful Computer Architecture teaching example.
 */

#if URG32_CONTROLLER_BRIDGE_ENABLE
static uint32_t bridge_state;

static uint32_t map_bridge_packet(const uint8_t *p, int n) {
    /*
     * Simple bridge packet format, easy to implement on any USB-host adapter:
     *   'U' 'G' lo hi
     * where lo/hi are the 16-bit URG32 button mask in little-endian order.
     */
    if (n >= 4 && p[0] == 'U' && p[1] == 'G') {
        return (uint32_t)p[2] | ((uint32_t)p[3] << 8);
    }
    return bridge_state;
}

void urg32_controller_bridge_init(void) {
    const uart_config_t cfg = {
        .baud_rate = URG32_CONTROLLER_BRIDGE_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    uart_driver_install(URG32_CONTROLLER_BRIDGE_UART, 256, 0, 0, NULL, 0);
    uart_param_config(URG32_CONTROLLER_BRIDGE_UART, &cfg);
    uart_set_pin(URG32_CONTROLLER_BRIDGE_UART,
                 URG32_PIN_CONTROLLER_TX,
                 URG32_PIN_CONTROLLER_RX,
                 UART_PIN_NO_CHANGE,
                 UART_PIN_NO_CHANGE);
}

static uint32_t read_bridge(void) {
    uint8_t b[16];
    int n = uart_read_bytes(URG32_CONTROLLER_BRIDGE_UART, b, sizeof(b), 0);
    if (n >= 4) bridge_state = map_bridge_packet(b, n);
    return bridge_state;
}
#else
void urg32_controller_bridge_init(void) {}
static uint32_t read_bridge(void) { return 0; }
#endif

static uint32_t read_gpio_buttons(void) {
    uint32_t v = 0;
    if (!gpio_get_level(URG32_PIN_BTN_LEFT))  v |= URG32_BTN_LEFT;
    if (!gpio_get_level(URG32_PIN_BTN_RIGHT)) v |= URG32_BTN_RIGHT;
    if (!gpio_get_level(URG32_PIN_BTN_UP))    v |= URG32_BTN_UP;
    if (!gpio_get_level(URG32_PIN_BTN_DOWN))  v |= URG32_BTN_DOWN;
    if (!gpio_get_level(URG32_PIN_BTN_A))     v |= URG32_BTN_A;
    if (!gpio_get_level(URG32_PIN_BTN_B))     v |= URG32_BTN_B;
    return v;
}

uint32_t urg32_controller_read(void) {
    uint32_t v = read_gpio_buttons();
    v |= read_bridge();
    return v;
}

const char *urg32_controller_name(uint32_t bit) {
    switch (bit) {
        case URG32_BTN_LEFT: return "LEFT";
        case URG32_BTN_RIGHT: return "RIGHT";
        case URG32_BTN_UP: return "UP";
        case URG32_BTN_DOWN: return "DOWN";
        case URG32_BTN_A: return "A / FIRE / SELECT";
        case URG32_BTN_B: return "B / BACK";
        case URG32_BTN_START: return "START / PAUSE";
        default: return "UNKNOWN";
    }
}
