# Deployment Guide

## Requirements

- ESP-IDF installed
- ESP32-C6 board
- ILI9341 SPI TFT display
- Buttons wired to GPIO with pull-ups
- Passive buzzer

## Build and flash

```bash
idf.py set-target esp32c6
idf.py build
idf.py flash monitor
```

## Select a game

Edit `main/urg32_config.h`:

```c
#define URG32_SELECTED_GAME URG32_GAME_PONG_GRAPHICS
#define URG32_DEFAULT_MODE URG32_MODE_UART_LCD_MIRROR
```

Pin assignments in `urg32_config.h` are reference values. Check your ESP32-C6 board pinout before wiring.
