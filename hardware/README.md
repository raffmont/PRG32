# PRG32 Reference Hardware

## Prototype breadboard parts

- ESP32-C6 development board
- ILI9341 2.8 inch SPI TFT, 320x240
- one 5-way digital joystick module for player 1
- optional second 5-way digital joystick module for player 2 games such as Pong
- one setup button, or use the joystick push switch when the wiring allows it
- Passive buzzer
- one MAX98357A I2S DAC/amplifier breakout and one 4-8 ohm speaker for mono
  PRG32 audio
- optional second MAX98357A and speaker for stereo PRG32 Audio Plus
- Jumper wires and breadboard

Desktop QEMU can emulate the PRG32 graphics viewport for early software tests,
but it does not replace this hardware validation. Use the physical board for
LCD wiring, GPIO buttons, buzzer/I2S output, and controller bridge wiring.

The resident firmware also starts the `PRG32` Wi-Fi AP for cartridge uploads.
Keep the antenna area of the ESP32-C6 module clear in the enclosure.

## USB controller bridge

ESP32-C6 remains the main RISC-V teaching microcontroller. It does not directly
host arbitrary wired USB HID controllers, so PRG32 uses a bridge:

```text
USB controller -> USB host bridge -> UART -> ESP32-C6
```

The bridge sends the packet documented in `docs/external_controllers.md`:

```text
'U' 'G' <lo> <hi>
```

Suitable bridge boards include ESP32-S3, RP2040 with USB host support,
CH559/CH554, or a PC-side serial helper during labs.

## Reference wiring

| ESP32-C6 signal | Display / device |
|---|---|
| 3V3 | TFT VCC |
| GND | TFT GND |
| GPIO7 | TFT MOSI |
| GPIO2 | TFT MISO / touch DO |
| GPIO6 | TFT SCLK |
| GPIO10 | TFT CS |
| GPIO8 | TFT DC |
| GPIO9 | TFT RST |
| GPIO5 | TFT BL |
| GPIO18 | P1 joystick LEFT to GND |
| GPIO19 | P1 joystick RIGHT to GND |
| GPIO4 | P1 joystick UP to GND |
| GPIO13 | P1 joystick DOWN to GND |
| GPIO14 | P1 START to GND |
| GPIO21 | P1 A / joystick select to GND |
| GPIO22 | P1 B / back to GND |
| GPIO15 | Passive buzzer |

## MAX98357A audio wiring

Mono audio uses one MAX98357A:

| ESP32-C6 signal | MAX98357A |
|---|---|
| 3V3 or 5V | VIN |
| GND | GND |
| GPIO4 default | BCLK |
| GPIO5 default | LRC / WS |
| GPIO6 default | DIN |
| GPIO7 default, optional | SD |

Stereo uses two MAX98357A boards. Both share BCLK, LRC/WS, DIN, power, and
ground. Configure one board for left-channel output and the other for
right-channel output using the breakout-specific jumper or mode pin.

The default audio GPIOs are Kconfig defaults for the audio examples. If a
classroom kit also uses those pins for display or joystick wiring, choose
non-conflicting audio pins in menuconfig before flashing.

Do not connect MAX98357A speaker outputs directly to headphones or line-level
inputs. Use 4-8 ohm speakers.

The previous six-tactile-button layout is replaced by a digital joystick module
such as the user-provided reference part:
<https://www.amazon.it/dp/B07HBPW3DF?ref=ppx_yo2ov_dt_b_fed_asin_title>.
Wire each joystick direction as a normally-open switch to ground; PRG32 enables
internal pull-ups.

For two-player games, mount a second joystick and assign its pins through
`PRG32_PIN_P2_LEFT`, `PRG32_PIN_P2_RIGHT`, `PRG32_PIN_P2_UP`,
`PRG32_PIN_P2_DOWN`, `PRG32_PIN_P2_A`, `PRG32_PIN_P2_B`, and
`PRG32_PIN_P2_START` in `main/prg32_config.h`.

The `kicad` directory contains starter placeholders for a KiCad production
board. The `case` directory contains a simple OpenSCAD enclosure starter. The
`prg32_v2` directory contains the next hardware revision scaffold.
