#ifndef PRG32_CONFIG_H
#define PRG32_CONFIG_H

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

#define PRG32_SELECTED_GAME PRG32_GAME_PONG_GRAPHICS

/* Runtime console mode. */
#define PRG32_MODE_UART_ONLY       0
#define PRG32_MODE_LCD_ONLY        1
#define PRG32_MODE_UART_LCD_MIRROR 2

#define PRG32_DEFAULT_MODE PRG32_MODE_UART_LCD_MIRROR

/* Reference ESP32-C6 pins. Change these for the selected development board. */
#define PRG32_PIN_LCD_MOSI   6
#define PRG32_PIN_LCD_MISO   7
#define PRG32_PIN_LCD_SCLK   5
#define PRG32_PIN_LCD_CS     10
#define PRG32_PIN_LCD_DC     11
#define PRG32_PIN_LCD_RST    12
#define PRG32_PIN_LCD_BL     13

#define PRG32_PIN_BTN_LEFT   0
#define PRG32_PIN_BTN_RIGHT  1
#define PRG32_PIN_BTN_UP     2
#define PRG32_PIN_BTN_DOWN   3
#define PRG32_PIN_BTN_A      4
#define PRG32_PIN_BTN_B      8
#define PRG32_PIN_BUZZER     9

/* Optional USB-controller support via an external USB HID host bridge. */
#define PRG32_CONTROLLER_BRIDGE_ENABLE 1
#define PRG32_CONTROLLER_BRIDGE_UART 1
#define PRG32_CONTROLLER_BRIDGE_BAUD 115200
#define PRG32_PIN_CONTROLLER_TX 18
#define PRG32_PIN_CONTROLLER_RX 19

/* Optional Wi-Fi score REST API. Fill credentials before flashing. */
#define PRG32_WIFI_SCORES_ENABLE 0
#define PRG32_WIFI_SSID "YOUR_WIFI_SSID"
#define PRG32_WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define PRG32_SCORE_MAX 16

#endif
