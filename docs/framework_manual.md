# PRG32 Framework Manual

PRG32 lets students write game logic in RISC-V assembly while a small C framework provides hardware access.

## Assembly ABI

Arguments use the standard RISC-V calling convention: `a0` to `a7` carry arguments and return values, `ra` holds the return address, and `sp` is kept 16-byte aligned around C calls.

## Console modes

- `PRG32_MODE_UART_ONLY`: serial terminal only.
- `PRG32_MODE_LCD_ONLY`: text appears on the ILI9341 display.
- `PRG32_MODE_UART_LCD_MIRROR`: debug text is sent both to serial and LCD.

## Graphics model

The physical display is 320x240, but the game viewport is 320x200. The extra vertical area is reserved for border, status, or debugging. The ILI9341 renderer tracks dirty rectangles and sends only changed areas over SPI using ESP-IDF SPI DMA.

The QEMU renderer exposes the same 320x200 PRG32 viewport through Espressif's virtual RGB panel. Student assembly code does not change; only the selected display backend changes.

Display backend selection:

- `CONFIG_PRG32_DISPLAY_ILI9341`: physical ILI9341 SPI TFT, default.
- `CONFIG_PRG32_DISPLAY_QEMU_RGB`: QEMU virtual RGB framebuffer.

Use the QEMU defaults file when running on a desktop:

```bash
idf.py -B build-qemu -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu qemu --graphics monitor
```

When the QEMU backend is selected, `main/prg32_config.h` disables physical GPIO
buttons, the buzzer, and the UART controller bridge. This keeps desktop screen
tests focused on rendering and GDB exercises.

## Tile engine

The tile engine exposes a 40x25 grid of 8x8 tiles. This matches a 320x200 retro screen exactly.

## Sprite engine

The sprite layer provides simple bitmap drawing and axis-aligned bounding-box collision detection.

## Audio

`prg32_audio_beep(hz, ms)` uses PWM to drive a passive buzzer.
