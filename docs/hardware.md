# PRG32 Hardware

This page summarizes the classroom reference wiring. See `hardware/README.md`
for the hardware directory map and board scaffolds.

## Base Hardware

| Quantity | Item | Purpose |
|---:|---|---|
| 1 | ESP32-C6 development board | PRG32 RISC-V host |
| 1 | ILI9341 SPI TFT or supported display | video output |
| 1 | digital joystick module | player 1 input |
| 1 | setup button | Wi-Fi setup mode |
| 1 | optional second joystick | player 2 games |

## Reference Display And Input Wiring

These pins match `main/prg32_config.h` for the ESP32-C6 physical ILI9341 build.
They are the same display wiring used by the Arduino/Adafruit validation sketch.

| ESP32-C6 | ILI9341 TFT / control |
|---|---|
| 3V3 | VCC |
| GND | GND |
| GPIO7 | MOSI |
| GPIO2 | MISO / touch DO |
| GPIO6 | SCLK |
| GPIO10 | CS |
| GPIO8 | DC |
| GPIO9 | RST |
| GPIO5 | BL |

| ESP32-C6 | Input |
|---|---|
| GPIO18 | P1 LEFT |
| GPIO19 | P1 RIGHT |
| GPIO4 | P1 UP |
| GPIO13 | P1 DOWN |
| GPIO14 | P1 START |
| GPIO21 | P1 A / joystick select |
| GPIO22 | P1 B / back |
| GPIO15 | passive buzzer |

The LCD backlight defaults to active-high. If a specific breakout uses an
active-low backlight transistor, set `PRG32_LCD_BACKLIGHT_ACTIVE_LEVEL` to `0`
in `main/prg32_config.h`.

## Mono Audio

| ESP32-C6 | MAX98357A |
|---|---|
| 3V3 or 5V | VIN |
| GND | GND |
| GPIO4 | BCLK |
| GPIO5 | LRC / WS |
| GPIO6 | DIN |
| GPIO7 optional | SD |

Connect one 4-8 ohm speaker to the MAX98357A speaker `+` and `-` outputs.

The default audio Kconfig pins overlap the reference display wiring. The
firmware splash therefore skips I2S welcome audio on this wiring unless the
audio pins are reassigned to unused GPIOs.

## Stereo Audio

Stereo uses two MAX98357A boards on the same I2S bus:

| ESP32-C6 | Left MAX98357A | Right MAX98357A |
|---|---|---|
| 3V3 or 5V | VIN | VIN |
| GND | GND | GND |
| GPIO4 | BCLK | BCLK |
| GPIO5 | LRC / WS | LRC / WS |
| GPIO6 | DIN | DIN |
| GPIO7 optional | SD | SD |

Configure the left board for left output and the right board for right output.
Breakout pin labels vary; verify the vendor pinout before soldering.

## Safety Notes

- Do not connect speaker outputs to headphones or line inputs.
- Keep speaker wires short during breadboard tests.
- Use a power source that can supply the speaker current.
- Reassign audio GPIOs if they conflict with the display or joystick wiring in
  a specific classroom kit.
