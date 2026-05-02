# Demo game

This directory contains two RISC-V assembly versions of the game:

- `ascii/game.S`: uses the PRG32 console API and runs in UART-only, LCD-only text, or mirrored mode.
- `graphics/game.S`: uses the 320x200 graphics API tuned for ILI9341.

Build one version by adding the corresponding source file to `main/CMakeLists.txt`,
or package it as a `.prg32` cartridge with `tools/prg32_game.py`.

In QEMU, input may be static unless a UART bridge is enabled.

The included source is deliberately commented line-by-line for classroom use.
