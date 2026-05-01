# USB Gamepad Bridge Hardware

Because ESP32-C6 is not a direct USB HID host for standard USB gamepads, PRG32 uses an optional bridge board.

## Reference bridge wiring

```text
USB gamepad -> USB-host bridge -> UART -> ESP32-C6 PRG32 board
```

| Bridge pin | ESP32-C6 pin | Notes |
|---|---|---|
| GND | GND | Common reference |
| TX | GPIO19 / controller RX | Bridge sends packets to C6 |
| RX | GPIO18 / controller TX | Optional for configuration |
| 3V3 | 3V3 | Only if current budget is safe |

Use a powered USB-host bridge when using controllers with LEDs, rumble, or high current draw.

## Packet generator pseudocode

```c
while (1) {
    uint16_t mask = read_usb_hid_gamepad_and_map_to_prg32();
    uart_write('U');
    uart_write('G');
    uart_write(mask & 0xff);
    uart_write(mask >> 8);
    delay_ms(8);
}
```

## Production PCB note

The KiCad reference board reserves a 4-pin connector named `J_GAMEPAD_BRIDGE` for the bridge UART. It keeps the ESP32-C6 board simple while allowing advanced labs to build or replace the bridge.
