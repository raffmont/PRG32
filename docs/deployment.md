# Deployment Guide

## Requirements

- ESP-IDF installed
- ESP32-C6 board
- ILI9341 SPI TFT display
- Buttons wired to GPIO with pull-ups
- Passive buzzer

For desktop screen testing instead of real hardware:

- ESP-IDF installed
- `qemu-riscv32` installed through `idf_tools.py`
- SDL2 and related QEMU runtime libraries on Linux/macOS

## Build and flash

```bash
idf.py set-target esp32c6
idf.py build
idf.py flash monitor
```

## Default app

The default firmware is a resident PRG32 runtime. With no cartridge installed it
prints `PRG32 Hello World`. When a cartridge is uploaded, the same firmware loads
the stored game from `cart0` and calls its `init`, `update`, and `draw` entries.

## Flash once, upload games

The physical build starts a Wi-Fi AP for cartridge upload:

```text
SSID: PRG32
Password: prg32game
URL: http://192.168.4.1
```

Build and upload a game:

```bash
python3 tools/prg32_game.py build \
  examples/games/asteroids/graphics/game.S \
  --firmware-elf build/PRG32.elf \
  --entry-prefix asteroids_graphics \
  --name asteroids \
  --out build/asteroids.prg32

python3 tools/prg32_game.py upload build/asteroids.prg32 --url http://192.168.4.1
```

See `docs/cartridges.md`.

## Build an example game

Example games live outside the default app in `examples/games`. During labs, add one source file to `main/CMakeLists.txt` and call its exported symbols from `main/main.c`.

Example game builds may still use `main/prg32_config.h` for display mode, pins, Wi-Fi credentials, and course-specific selection macros:

```c
#define PRG32_SELECTED_GAME PRG32_GAME_PONG_GRAPHICS
#define PRG32_DEFAULT_MODE PRG32_MODE_UART_LCD_MIRROR
```

Pin assignments in `prg32_config.h` are reference values. Check your ESP32-C6 board pinout before wiring.

## Run in QEMU with a virtual screen

QEMU uses a separate build directory and `sdkconfig.defaults.qemu` so the
physical board build remains unchanged. The emulator target is ESP32-C3 because
that is Espressif's documented RISC-V QEMU graphics target; PRG32 hardware
deployment still targets ESP32-C6.

```bash
idf.py -B build-qemu -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu set-target esp32c3
idf.py -B build-qemu -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu qemu --graphics monitor
```

The QEMU build selects `CONFIG_PRG32_DISPLAY_QEMU_RGB` and renders the PRG32
320x200 game viewport with Espressif's virtual RGB panel. The normal hardware
build keeps the ESP32-C6 target and ILI9341 SPI display backend.

QEMU cartridge testing uses the same `.prg32` package but stages it into
`build-qemu/qemu_flash.bin`:

```bash
python3 tools/prg32_game.py upload-qemu build-qemu/asteroids.prg32
```

Shortcut scripts:

- macOS/Linux: `tools/qemu.sh`
- Windows PowerShell: `tools/qemu.ps1`

See `docs/qemu.md` for host-specific setup and troubleshooting.
