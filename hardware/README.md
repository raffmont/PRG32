# URG32 Reference Hardware

## Prototype breadboard parts

- ESP32-C6 development board
- ILI9341 2.8 inch SPI TFT, 320x240
- 6 tactile buttons
- Passive buzzer
- Jumper wires and breadboard

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

The `kicad` directory contains starter placeholders for a KiCad production board. The `case` directory contains a simple OpenSCAD enclosure starter.
