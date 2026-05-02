# PRG32 - University of Naples Parthenope 32-bit RISC-V Gaming Platform

PRG32 is an educational retro-gaming platform for teaching RISC-V assembly to first-year Computer Science and Computer Engineering students.

The firmware targets the ESP32-C6 RISC-V microcontroller and an ILI9341 320x240 SPI TFT display. The same game ABI can also run in Espressif QEMU with a virtual RGB screen window for desktop testing on macOS, Windows, and Linux. The C framework hides hardware setup while exposing a small ABI that assembly programs can call directly.

## Repository Layout

- `components/prg32`: reusable PRG32 framework component.
- `main`: minimal framework smoke-test app.
- `examples/games`: external RISC-V assembly game examples.
- `docs`: API notes, tutorials, labs, and debugging assignments.
- `docs/qemu.md`: desktop QEMU screen emulator setup.
- `tools/prg32_score_server`: optional Flask + SQLite classroom scoreboard.
- `.vscode` and `PRG32.code-workspace`: student-ready VS Code setup.
- `hardware`: breadboard notes, USB-controller bridge notes, PCB/enclosure starters.

## Quick Start

Open `PRG32.code-workspace` in VS Code, install the recommended extensions, then use the provided tasks:

```bash
idf.py set-target esp32c6
idf.py build
idf.py flash monitor
```

The default app prints `PRG32 Hello World` through the framework console. Example games are intentionally external to the firmware app so students can add one game at a time during labs.

## QEMU Screen Quick Start

After installing ESP-IDF 5.3 or newer and `qemu-riscv32`, run PRG32 with the virtual RGB screen. The emulator build uses ESP32-C3 because that is Espressif's maintained RISC-V QEMU graphics target; the physical board build remains ESP32-C6.

```bash
idf.py -B build-qemu -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu set-target esp32c3
idf.py -B build-qemu -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu qemu --graphics monitor
```

Unix-like hosts can also run `tools/qemu.sh`; Windows PowerShell users can run
`tools/qemu.ps1` from an ESP-IDF shell.

In VS Code, use:

- `PRG32: qemu set target esp32c3`
- `PRG32: qemu build`
- `PRG32: qemu screen`
- `PRG32: qemu debug server` and `PRG32: qemu gdb` for register tracing

See `docs/qemu.md` for macOS, Windows, and Linux setup details.

## Framework Modes

1. UART terminal only.
2. LCD-only classroom mode.
3. UART + LCD mirrored debug mode.

Configure pins, console mode, controller bridge settings, and Wi-Fi score support in `main/prg32_config.h`. Select the physical ILI9341 or QEMU RGB display backend with ESP-IDF Kconfig; `sdkconfig.defaults.qemu` enables the QEMU backend.

## Example Games

The included assembly examples live in `examples/games`:

- Pong
- Breakout
- Space Invaders
- Pac-Man inspired maze game
- Asteroids-inspired ship demo

Each game has ASCII and graphics versions and is commented line-by-line for classroom use.

## Course Material

Start with `docs/tutorial.md`, then use the labs in `docs/labs`:

- Step-by-step build, input, graphics, sound, scores, and controller labs.
- Register tracing and memory inspection exercises.
- "Break the game and fix it" assignments.

## APIs

- Score API: `docs/score_api.md`
- External controllers: `docs/external_controllers.md`
- Framework manual: `docs/framework_manual.md`
- QEMU screen emulator: `docs/qemu.md`

## Hardware

PRG32 supports GPIO buttons and an optional USB-HID bridge:

```text
USB controller -> USB host bridge -> UART packet -> ESP32-C6 -> PRG32 input bitmask
```

See `hardware/README.md` and `hardware/prg32_v2/README.md`.
