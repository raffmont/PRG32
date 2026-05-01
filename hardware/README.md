# PRG32 Reference Hardware

## Prototype breadboard parts

- ESP32-C6 development board
- ILI9341 2.8 inch SPI TFT, 320x240
- 6 tactile buttons
- Passive buzzer
- Jumper wires and breadboard

## USB controller bridge

ESP32-C6 remains the main RISC-V teaching microcontroller. It does not directly host arbitrary wired USB HID controllers, so PRG32 uses a bridge:

```text
USB controller -> USB host bridge -> UART -> ESP32-C6
```

The bridge sends the packet documented in `docs/external_controllers.md`:

```text
'U' 'G' <lo> <hi>
```

Suitable bridge boards include ESP32-S3, RP2040 with USB host support, CH559/CH554, or a PC-side serial helper during labs.

## Reference wiring

| ESP32-C6 signal | Display / device |
|---|---|
| 3V3 | TFT VCC |
| GND | TFT GND |
| GPIO6 | TFT MOSI |
| GPIO7 | TFT MISO / touch DO |
| GPIO5 | TFT SCLK |
| GPIO10 | TFT CS |
| GPIO11 | TFT DC |
| GPIO12 | TFT RST |
| GPIO13 | TFT BL |
| GPIO0 | Left button to GND |
| GPIO1 | Right button to GND |
| GPIO2 | Up button to GND |
| GPIO3 | Down button to GND |
| GPIO4 | A button to GND |
| GPIO8 | B button to GND |
| GPIO9 | Passive buzzer |

The `kicad` directory contains starter placeholders for a KiCad production board. The `case` directory contains a simple OpenSCAD enclosure starter. The `prg32_v2` directory contains the next hardware revision scaffold.
