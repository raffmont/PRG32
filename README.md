# PRG32

PRG32 is a teaching platform for first-year RISC-V assembly labs based on ESP32-C6.
Students write assembly games against a small C ABI (`components/prg32/include/prg32.h`) and run them on:

- ESP32-C6 + ILI9341 TFT (physical board)
- Espressif QEMU graphics mode (desktop, ESP32-C3 target)

## What Is In This Repo

- `components/prg32`: PRG32 runtime API (display, input, audio, cartridge loader, optional Wi-Fi HTTP API)
- `main`: default firmware app (resident runtime + cartridge loop)
- `examples/games`: assembly demos (`ascii/game.S` and `graphics/game.S`)
- `tools/prg32_game.py`: cartridge build/upload/staging tool (`.prg32`)
- `tools/prg32_score_server`: optional Flask + SQLite score backend
- `docs`: tutorial, labs, API/manual, QEMU and cartridge workflows

## Requirements

- ESP-IDF 5.3+ in shell (`idf.py` available)
- RISC-V toolchain from ESP-IDF (`riscv32-esp-elf-*`)
- Python 3.9+ for helper tools
- For QEMU graphics: `qemu-riscv32` installed via IDF tools

## Build And Run (Hardware)

```bash
idf.py set-target esp32c6
idf.py build
idf.py flash monitor
```

Default app behavior:

- prints `PRG32 Hello World`
- starts resident runtime
- runs stored cartridge if one exists

## Build And Run (QEMU)

```bash
idf.py -B build-qemu -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu set-target esp32c3
idf.py -B build-qemu -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu qemu --graphics monitor
```

Shortcut scripts:

- macOS/Linux: `tools/qemu.sh`
- Windows PowerShell: `tools/qemu.ps1`

## Run Demo Games

There are two supported flows.

1. Cartridge flow (recommended):

```bash
python3 tools/prg32_game.py build \
  examples/games/asteroids/graphics/game.S \
  --firmware-elf build/PRG32.elf \
  --entry-prefix asteroids_graphics \
  --name asteroids \
  --out build/asteroids.prg32
```

Hardware upload:

```bash
python3 tools/prg32_game.py upload build/asteroids.prg32 --url http://192.168.4.1
```

QEMU staging:

```bash
python3 tools/prg32_game.py upload-qemu build/asteroids.prg32 --flash build-qemu/qemu_flash.bin
```

2. Lab flow (compile game into app):
- Add one example `game.S` in `main/CMakeLists.txt`
- Call its `<name>_init`, `<name>_update`, `<name>_draw` from `main/main.c`

## Network Defaults (Physical Cartridge Upload)

```text
SSID: PRG32
Password: prg32game
URL: http://192.168.4.1
```

## Documentation Map

- `docs/tutorial.md`: end-to-end student tutorial
- `docs/framework_manual.md`: ABI and framework behavior
- `docs/cartridges.md`: flash-once cartridge workflow
- `docs/qemu.md`: QEMU setup and troubleshooting
- `docs/score_api.md`: score endpoints and client flow
- `docs/external_controllers.md`: UART controller bridge protocol
- `docs/labs`: classroom lab sequence and debugging exercises

## Screenshots

Place screenshots in `docs/images/` (create if missing) and reference them here:

- `docs/images/prg32-qemu-hello-world.png`
- `docs/images/prg32-asteroids-graphics.png`
- `docs/images/prg32-runtime-api-response.png`
- `docs/images/prg32-cartridge-upload-ok.png`
