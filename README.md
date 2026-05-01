# URG32 - Uniparthenope RISC Gaming platform at 32 bits

URG32 is an educational retro-gaming platform for teaching RISC-V assembly to first-year Computer Science and Computer Engineering students.

The project targets the ESP32-C6 RISC-V microcontroller and the ILI9341 2.8 inch SPI TFT display. It provides a C framework that hides hardware setup while exposing a small, stable ABI callable from RISC-V assembly.

## Modes

1. **Headless UART terminal**: ASCII games run over serial.
2. **LCD-only classroom mode**: on-screen text console and 320x200 graphics viewport.
3. **UART + LCD mirrored debug mode**: serial output is mirrored to the display.

## Included demos

Each demo has ASCII and graphics versions:

- Pong
- Breakout
- Space Invaders
- Pac-Man inspired maze game

## Hardware target

- ESP32-C6 development board
- ILI9341 320x240 SPI TFT display
- GPIO buttons or joystick
- Passive buzzer driven by PWM

## Build

```bash
idf.py set-target esp32c6
idf.py menuconfig
idf.py build
idf.py flash monitor
```

Select the game and video mode in `main/urg32_config.h`.

## Status

This kit is designed as a complete educational scaffold. Hardware pin assignments may need adjustment for the specific ESP32-C6 board and ILI9341 module used in class.
