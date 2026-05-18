#ifndef PRG32_CONFIG_H
#define PRG32_CONFIG_H

#include "sdkconfig.h"

/* Example game identifiers used by lab builds that opt into examples/games. */
#define PRG32_GAME_PONG_ASCII          1
#define PRG32_GAME_PONG_GRAPHICS       2
#define PRG32_GAME_BREAKOUT_ASCII      3
#define PRG32_GAME_BREAKOUT_GRAPHICS   4
#define PRG32_GAME_INVADERS_ASCII      5
#define PRG32_GAME_INVADERS_GRAPHICS   6
#define PRG32_GAME_PACMAN_ASCII        7
#define PRG32_GAME_PACMAN_GRAPHICS     8
#define PRG32_GAME_ASTEROIDS_ASCII     9
#define PRG32_GAME_ASTEROIDS_GRAPHICS  10
#define PRG32_GAME_TETRIS_ASCII        11
#define PRG32_GAME_TETRIS_GRAPHICS     12
#define PRG32_GAME_TETRIS_C            13
#define PRG32_GAME_PLATFORMER_ASCII    14
#define PRG32_GAME_PLATFORMER_GRAPHICS 15
#define PRG32_GAME_PLATFORMER_C        16
#define PRG32_GAME_PONG_C              17
#define PRG32_GAME_BREAKOUT_C          18
#define PRG32_GAME_INVADERS_C          19
#define PRG32_GAME_PACMAN_C            20
#define PRG32_GAME_ASTEROIDS_C         21

/* Runtime console mode. */
#define PRG32_MODE_UART_ONLY       0
#define PRG32_MODE_LCD_ONLY        1
#define PRG32_MODE_UART_LCD_MIRROR 2

#define PRG32_DEFAULT_MODE PRG32_MODE_UART_LCD_MIRROR

#ifndef PRG32_DEBUG
#define PRG32_DEBUG 0
#endif

#if CONFIG_PRG32_DISPLAY_QEMU_RGB
/* QEMU screen builds do not touch physical board pins. */
#define PRG32_PIN_LCD_MOSI   -1
#define PRG32_PIN_LCD_MISO   -1
#define PRG32_PIN_LCD_SCLK   -1
#define PRG32_PIN_LCD_CS     -1
#define PRG32_PIN_LCD_DC     -1
#define PRG32_PIN_LCD_RST    -1
#define PRG32_PIN_LCD_BL     -1

#define PRG32_PIN_BTN_LEFT   -1
#define PRG32_PIN_BTN_RIGHT  -1
#define PRG32_PIN_BTN_UP     -1
#define PRG32_PIN_BTN_DOWN   -1
#define PRG32_PIN_BTN_A      -1
#define PRG32_PIN_BTN_B      -1
#define PRG32_PIN_BTN_START  -1
#define PRG32_PIN_P2_LEFT    -1
#define PRG32_PIN_P2_RIGHT   -1
#define PRG32_PIN_P2_UP      -1
#define PRG32_PIN_P2_DOWN    -1
#define PRG32_PIN_P2_A       -1
#define PRG32_PIN_P2_B       -1
#define PRG32_PIN_P2_START   -1
#define PRG32_PIN_SETUP      -1
#define PRG32_PIN_BUZZER     -1
#define PRG32_BOOT_DIAGNOSTIC_DELAY_MS 0

#define PRG32_CONTROLLER_BRIDGE_ENABLE 0
#define PRG32_CONTROLLER_BRIDGE_UART 1
#define PRG32_CONTROLLER_BRIDGE_BAUD 115200
#define PRG32_PIN_CONTROLLER_TX -1
#define PRG32_PIN_CONTROLLER_RX -1

#define PRG32_GAME_UPLOAD_ENABLE 0
#else
/*
 * ESP32-C6 breadboard wiring for the physical ILI9341 build.
 * These pins match the classroom display harness tested with Arduino/Adafruit.
 */
#define PRG32_PIN_LCD_MOSI   7
#define PRG32_PIN_LCD_MISO   2
#define PRG32_PIN_LCD_SCLK   6
#define PRG32_PIN_LCD_CS     10
#define PRG32_PIN_LCD_DC     8
#define PRG32_PIN_LCD_RST    9
#define PRG32_PIN_LCD_BL     5

#define PRG32_LCD_SPI_CLOCK_HZ 10000000
#define PRG32_LCD_BACKLIGHT_ACTIVE_LEVEL 1
#define PRG32_LCD_BOOT_TEST_MS 0
#define PRG32_LCD_SOFT_SPI 0
#define PRG32_BOOT_SIGNAL_ENABLE 0
#define PRG32_BOOT_DIAGNOSTIC_DELAY_MS 0

#define PRG32_PIN_BTN_UP     4
#define PRG32_PIN_BTN_DOWN   13
#define PRG32_PIN_BTN_LEFT   18
#define PRG32_PIN_BTN_RIGHT  19
#define PRG32_PIN_BTN_START  14
#define PRG32_PIN_BTN_A      21
#define PRG32_PIN_BTN_B      22

#define PRG32_PIN_SETUP      -1

/* Optional second digital joystick. Leave pins at -1 when not mounted. */
#define PRG32_PIN_P2_LEFT    -1
#define PRG32_PIN_P2_RIGHT   -1
#define PRG32_PIN_P2_UP      -1
#define PRG32_PIN_P2_DOWN    -1
#define PRG32_PIN_P2_A       -1
#define PRG32_PIN_P2_B       -1
#define PRG32_PIN_P2_START   -1
#define PRG32_PIN_BUZZER     15

/* Optional USB-controller support via an external USB HID host bridge. */
#define PRG32_CONTROLLER_BRIDGE_ENABLE 0
#define PRG32_CONTROLLER_BRIDGE_UART 1
#define PRG32_CONTROLLER_BRIDGE_BAUD 115200
#define PRG32_PIN_CONTROLLER_TX 16
#define PRG32_PIN_CONTROLLER_RX 17
#endif

/*
 * Uploadable game cartridges.
 *
 * Physical-board builds enable a small SoftAP + HTTP API by default so the
 * firmware can be flashed once and games can be replaced from a host tool.
 * QEMU builds keep Wi-Fi disabled and use tools/prg32_game.py to patch the
 * emulator flash image.
 */
#ifndef PRG32_GAME_UPLOAD_ENABLE
#define PRG32_GAME_UPLOAD_ENABLE 1
#endif

/* Optional Wi-Fi score REST API. Fill credentials before flashing. */
#define PRG32_WIFI_SCORES_ENABLE 0
#define PRG32_WIFI_SSID "YOUR_WIFI_SSID"
#define PRG32_WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define PRG32_SCORE_MAX 16
#define PRG32_IDLE_HEARTBEAT_MS 5000

#define PRG32_WIFI_STA_ENABLE PRG32_WIFI_SCORES_ENABLE
#define PRG32_WIFI_AP_ENABLE PRG32_GAME_UPLOAD_ENABLE
#define PRG32_WIFI_ENABLE (PRG32_WIFI_STA_ENABLE || PRG32_WIFI_AP_ENABLE)
#define PRG32_BOOT_SETUP_MODE PRG32_WIFI_ENABLE
#define PRG32_WIFI_AP_SSID "PRG32"
#define PRG32_WIFI_AP_PASSWORD "prg32game"
#define PRG32_WIFI_AP_CHANNEL 6
#define PRG32_WIFI_AP_MAX_CONN 4

#endif
