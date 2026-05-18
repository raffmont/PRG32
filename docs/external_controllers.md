# PRG32 USB Game Controller Support

## Important ESP32-C6 limitation

ESP32-C6 provides USB Serial/JTAG functionality for connection to a host PC. It
is not a general-purpose USB host port for directly plugging in standard USB HID
game controllers. PRG32 therefore supports USB controllers through an
**external USB-host bridge**.

Recommended bridge choices:

- ESP32-S3 running a USB HID host bridge firmware.
- RP2040 with USB host capability or a USB-host shield.
- CH559/CH554 USB-host microcontroller.
- A PC-side helper during laboratories, forwarding keyboard/gamepad state over serial.

The C6 game code does not change. It reads the same PRG32 button bitmask.

## PRG32 button map

| PRG32 bit | Meaning | Gamepad mapping |
|---|---|---|
| `PRG32_BTN_LEFT` | Move left | D-pad left / left stick X negative |
| `PRG32_BTN_RIGHT` | Move right | D-pad right / left stick X positive |
| `PRG32_BTN_UP` | Move up | D-pad up / left stick Y negative |
| `PRG32_BTN_DOWN` | Move down | D-pad down / left stick Y positive |
| `PRG32_BTN_A` | Fire / select / confirm | South button, often A/Cross |
| `PRG32_BTN_B` | Back / secondary action | East button, often B/Circle |
| `PRG32_BTN_START` | Pause / start | Start/Menu button |

Player 2 uses the same layout shifted to bits 8-14:

| PRG32 bit | Meaning |
|---|---|
| `PRG32_P2_BTN_LEFT` | Player 2 left |
| `PRG32_P2_BTN_RIGHT` | Player 2 right |
| `PRG32_P2_BTN_UP` | Player 2 up |
| `PRG32_P2_BTN_DOWN` | Player 2 down |
| `PRG32_P2_BTN_A` | Player 2 confirm/action |
| `PRG32_P2_BTN_B` | Player 2 back/secondary action |
| `PRG32_P2_BTN_START` | Player 2 start |

## UART bridge packet

The bridge sends a compact packet to the ESP32-C6:

```text
byte 0: 'U'
byte 1: 'G'
byte 2: low byte of button mask
byte 3: high byte of button mask
```

Example: LEFT + A = `0x0001 | 0x0010 = 0x0011`, so the packet is:

```text
'U' 'G' 0x11 0x00
```

Example: player 2 LEFT is bit 8, so the packet is:

```text
'U' 'G' 0x00 0x01
```

## Framework configuration

In `main/prg32_config.h`:

```c
#define PRG32_CONTROLLER_BRIDGE_ENABLE 1
#define PRG32_CONTROLLER_BRIDGE_UART 1
#define PRG32_CONTROLLER_BRIDGE_BAUD 115200
#define PRG32_PIN_CONTROLLER_TX 16
#define PRG32_PIN_CONTROLLER_RX 17
```

The function `prg32_input_read()` merges GPIO buttons and the bridge state, so
local arcade buttons and external USB controllers can be used at the same time.
The bridge is disabled in the default physical joystick build to avoid a
floating UART RX pin in student wiring; enable it only when a bridge is wired.

## Teaching note

This layer is deliberately similar to memory-mapped console input: the assembly
game sees one integer register-like value and tests bits with `andi`.
