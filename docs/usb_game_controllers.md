# URG32 USB Game Controller Support

## Important ESP32-C6 limitation

ESP32-C6 provides USB Serial/JTAG functionality for connection to a host PC. It is not a general-purpose USB host port for directly plugging in standard USB HID game controllers. URG32 therefore supports USB controllers through an **external USB-host bridge**.

Recommended bridge choices:

- ESP32-S3 running a USB HID host bridge firmware.
- RP2040 with USB host capability or a USB-host shield.
- CH559/CH554 USB-host microcontroller.
- A PC-side helper during laboratories, forwarding keyboard/gamepad state over serial.

The C6 game code does not change. It reads the same URG32 button bitmask.

## URG32 button map

| URG32 bit | Meaning | Gamepad mapping |
|---|---|---|
| `URG32_BTN_LEFT` | Move left | D-pad left / left stick X negative |
| `URG32_BTN_RIGHT` | Move right | D-pad right / left stick X positive |
| `URG32_BTN_UP` | Move up | D-pad up / left stick Y negative |
| `URG32_BTN_DOWN` | Move down | D-pad down / left stick Y positive |
| `URG32_BTN_A` | Fire / select / confirm | South button, often A/Cross |
| `URG32_BTN_B` | Back / secondary action | East button, often B/Circle |
| `URG32_BTN_START` | Pause / start | Start/Menu button |

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

## Framework configuration

In `main/urg32_config.h`:

```c
#define URG32_CONTROLLER_BRIDGE_ENABLE 1
#define URG32_CONTROLLER_BRIDGE_UART 1
#define URG32_CONTROLLER_BRIDGE_BAUD 115200
#define URG32_PIN_CONTROLLER_TX 18
#define URG32_PIN_CONTROLLER_RX 19
```

The function `urg32_input_read()` merges GPIO buttons and the bridge state, so local arcade buttons and external USB controllers can be used at the same time.

## Teaching note

This layer is deliberately similar to memory-mapped console input: the assembly game sees one integer register-like value and tests bits with `andi`.
