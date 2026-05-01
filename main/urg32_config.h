#ifndef URG32_CONFIG_H
#define URG32_CONFIG_H

/* Select one game implementation at compile time. */
#define URG32_GAME_PONG_ASCII          1
#define URG32_GAME_PONG_GRAPHICS       2
#define URG32_GAME_BREAKOUT_ASCII      3
#define URG32_GAME_BREAKOUT_GRAPHICS   4
#define URG32_GAME_INVADERS_ASCII      5
#define URG32_GAME_INVADERS_GRAPHICS   6
#define URG32_GAME_PACMAN_ASCII        7
#define URG32_GAME_PACMAN_GRAPHICS     8

#define URG32_SELECTED_GAME URG32_GAME_PONG_GRAPHICS

/* Runtime console mode. */
#define URG32_MODE_UART_ONLY       0
#define URG32_MODE_LCD_ONLY        1
#define URG32_MODE_UART_LCD_MIRROR 2

#define URG32_DEFAULT_MODE URG32_MODE_UART_LCD_MIRROR

/* Reference ESP32-C6 pins. Change these for the selected development board. */
#define URG32_PIN_LCD_MOSI   6
#define URG32_PIN_LCD_MISO   7
#define URG32_PIN_LCD_SCLK   5
#define URG32_PIN_LCD_CS     10
#define URG32_PIN_LCD_DC     11
#define URG32_PIN_LCD_RST    12
#define URG32_PIN_LCD_BL     13

#define URG32_PIN_BTN_LEFT   0
#define URG32_PIN_BTN_RIGHT  1
#define URG32_PIN_BTN_UP     2
#define URG32_PIN_BTN_DOWN   3
#define URG32_PIN_BTN_A      4
#define URG32_PIN_BTN_B      8
#define URG32_PIN_BUZZER     9

#endif
