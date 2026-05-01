# Demo game

This directory contains two RISC-V assembly versions of the game:

- `ascii/game.S`: uses the PRG32 console API and runs in UART-only, LCD-only text, or mirrored mode.
- `graphics/game.S`: uses the 320x200 graphics API tuned for ILI9341.

Select the version in `main/prg32_config.h`, then build with ESP-IDF.

The included source is deliberately commented line-by-line for classroom use.
