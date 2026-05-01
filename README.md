# PRG32 - University of Naples Parthenope 32-bit RISC-V Gaming Platform

PRG32 is an educational retro-gaming platform for teaching RISC-V assembly to first-year Computer Science and Computer Engineering students.

The firmware targets the ESP32-C6 RISC-V microcontroller and an ILI9341 320x240 SPI TFT display. The C framework hides hardware setup while exposing a small ABI that assembly programs can call directly.

## Repository Layout

- `components/prg32`: reusable PRG32 framework component.
- `main`: minimal framework smoke-test app.
- `examples/games`: external RISC-V assembly game examples.
- `docs`: API notes, tutorials, labs, and debugging assignments.
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

## Framework Modes

1. UART terminal only.
2. LCD-only classroom mode.
3. UART + LCD mirrored debug mode.

Configure pins, display mode, controller bridge settings, and Wi-Fi score support in `main/prg32_config.h`.

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

## Hardware

PRG32 supports GPIO buttons and an optional USB-HID bridge:

```text
USB controller -> USB host bridge -> UART packet -> ESP32-C6 -> PRG32 input bitmask
```

See `hardware/README.md` and `hardware/prg32_v2/README.md`.
