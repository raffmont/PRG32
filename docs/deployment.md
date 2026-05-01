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

## Default app

The default firmware is a framework smoke test that prints `PRG32 Hello World`.

## Build an example game

Example games live outside the default app in `examples/games`. During labs, add one source file to `main/CMakeLists.txt` and call its exported symbols from `main/main.c`.

Example game builds may still use `main/prg32_config.h` for display mode, pins, Wi-Fi credentials, and course-specific selection macros:

```c
#define PRG32_SELECTED_GAME PRG32_GAME_PONG_GRAPHICS
#define PRG32_DEFAULT_MODE PRG32_MODE_UART_LCD_MIRROR
```

Pin assignments in `prg32_config.h` are reference values. Check your ESP32-C6 board pinout before wiring.
