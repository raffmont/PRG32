# PRG32 v2 Hardware Scaffold

This directory is the starting point for the next reference board and enclosure.

## Design Goals

- ESP32-C6 as the main RISC-V teaching CPU.
- ILI9341 SPI display header.
- Six local buttons and passive buzzer.
- UART header for USB-HID bridge input.
- USB-C power and programming through the development board or carrier.
- Exposed test pads for UART, SPI, reset, boot, and buzzer PWM.

## Bridge Topology

```text
USB game controller
        |
        v
USB host bridge MCU
        |
        v
UART packet: 'U' 'G' lo hi
        |
        v
ESP32-C6 PRG32 firmware
```

## Files

- `PRG32_v2.kicad_pro`: KiCad project placeholder.
- `enclosure.scad`: OpenSCAD starter enclosure.

The board is intentionally scaffolded rather than finalized so students can inspect and extend the design during hardware labs.
