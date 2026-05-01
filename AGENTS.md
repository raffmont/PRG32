# AGENTS.md

Guidance for coding agents working in this repository.

## Scope

This file applies to the whole repository. Follow it unless a more specific
`AGENTS.md` exists deeper in the tree.

PRG32 is an educational ESP32-C6 / RISC-V assembly gaming platform. The project
is deliberately simple, readable, and classroom-oriented. Preserve that spirit:
prefer explicit code, clear names, and teachable examples over clever compact
abstractions.

## Project Snapshot

- Target board: ESP32-C6.
- Display: ILI9341 SPI TFT, 320x240 physical, 320x200 game viewport.
- Framework component: `components/prg32`.
- Default firmware app: `main`, a minimal PRG32 Hello World smoke test.
- Assembly game examples: `examples/games`.
- Course materials: `docs`, especially `docs/labs`.
- Optional score server: `tools/prg32_score_server`.
- Student VS Code setup: `.vscode` and `PRG32.code-workspace`.
- Hardware notes and v2 scaffold: `hardware`.

## Repository Layout

```text
.
|-- components/prg32/              ESP-IDF component implementing the PRG32 API
|   |-- include/prg32.h             Public C/assembly ABI
|   |-- prg32_*.c                   Framework implementation files
|   `-- CMakeLists.txt              Component registration
|-- main/                           Minimal firmware app and config
|   |-- main.c                      Default Hello World app
|   |-- prg32_config.h              Board pins and feature flags
|   `-- CMakeLists.txt
|-- examples/games/                 External assembly demos, not default build
|-- docs/                           Manuals, tutorial, labs, debugging exercises
|-- hardware/                       Wiring, USB bridge, PCB/enclosure scaffold
|-- tools/prg32_score_server/       Flask + SQLite REST score service
|-- .vscode/                        Student-ready VS Code tasks/settings
`-- PRG32.code-workspace
```

Do not reintroduce the old `components/urg32`, `main/urg32_config.h`, or top-level
`games/` layout. The canonical names are `prg32` and `PRG32`.

## Naming Rules

- Public C functions use `prg32_` prefix.
- Public macros and constants use `PRG32_` prefix.
- Component files use `components/prg32/prg32_*.c`.
- Include the framework as `#include "prg32.h"`.
- Include board configuration as `#include "prg32_config.h"`.
- Example game symbols should be unique and game-specific, for example:
  - `asteroids_graphics_init`
  - `asteroids_graphics_update`
  - `asteroids_graphics_draw`

Avoid generic `game_init`, `game_update`, and `game_draw` symbols in examples
unless a lab explicitly asks students to create a wrapper.

## Build Model

The default app is intentionally small:

```c
#include "prg32.h"

void app_main(void) {
    prg32_init();
    prg32_console_clear();
    prg32_console_write("PRG32 Hello World\n");
    while (1) {
        prg32_gfx_present();
    }
}
```

The default `main/CMakeLists.txt` should keep games out of the app:

```cmake
idf_component_register(
    SRCS "main.c"
    REQUIRES prg32
    INCLUDE_DIRS "."
)
```

Labs may temporarily add a specific example game source to `main/CMakeLists.txt`,
but the committed default should stay decoupled unless the user explicitly asks
to turn a game into the default firmware app.

## ESP-IDF Commands

Use these when ESP-IDF is available in the shell:

```bash
idf.py set-target esp32c6
idf.py build
idf.py flash monitor
```

Useful checks:

```bash
git diff --check
python3 -m py_compile tools/prg32_score_server/app.py
```

This environment may not have `idf.py`, `IDF_PATH`, or `riscv32-esp-elf-gcc` on
`PATH`. If they are missing, say so clearly. Do not claim a full firmware build
was run unless `idf.py build` actually succeeded.

## C Framework Guidelines

The public ABI is `components/prg32/include/prg32.h`. Keep it small, stable, and
friendly to RISC-V assembly callers.

When editing framework code:

- Keep dependencies in `components/prg32/CMakeLists.txt`.
- Keep app-specific pin and feature config in `main/prg32_config.h`.
- Preserve `prg32_init()` as the one-call framework initializer.
- Do not expose ESP-IDF-only types in the public ABI unless absolutely needed.
- Return simple `int` status codes for APIs called from assembly.
- Check pointer inputs in helpers that can be called from student code.
- Keep comments short and educational where they clarify hardware or ABI behavior.

Important implementation details:

- ILI9341 receives RGB565 bytes in wire order; do not regress byte swapping.
- The game viewport is `PRG32_GAME_W` x `PRG32_GAME_H` = 320x200.
- Console text must remain visible on LCD and UART mirror modes.
- `prg32_input_read()` merges local GPIO buttons and optional UART bridge state.
- The controller UART packet is:

```text
'U' 'G' <low byte> <high byte>
```

## Assembly Example Guidelines

Example assembly is teaching material. It should be verbose and heavily
commented enough for first-year students.

Every example game directory should contain:

```text
examples/games/<name>/
|-- README.md
|-- ascii/game.S
`-- graphics/game.S
```

When adding a game:

1. Add both `ascii/game.S` and `graphics/game.S` unless the user asks for only one.
2. Add a local `README.md` with controls and learning goals.
3. Update `examples/games/README.md`.
4. Update the example list in `README.md`.
5. Add `PRG32_GAME_<NAME>_ASCII` and `PRG32_GAME_<NAME>_GRAPHICS` identifiers in
   `main/prg32_config.h` if the examples list uses selection constants.

Assembly conventions:

- Use `.option norelax` at the top of RISC-V example files.
- Save and restore `ra` around any call into C.
- Keep stack alignment at 16 bytes around C calls.
- Prefer `t0`-`t6` for temporary values that do not need to survive C calls.
- Use `s0`-`s11` only when saving/restoring them in the stack frame.
- Avoid relying on `t` or `a` registers surviving a `call`.
- Use named local labels such as `.Lasteroids_done` for larger routines.
- If numeric local labels are used, keep them simple and readable.
- Do not make the examples too feature-rich; each one should demonstrate a few
  concepts clearly: input bitmasking, memory variables, drawing, sound, and calls.

## Documentation Guidelines

Docs are part of the teaching product. Keep them current when changing APIs,
paths, or example names.

Core docs:

- `README.md`: top-level orientation.
- `docs/tutorial.md`: end-to-end tutorial.
- `docs/framework_manual.md`: PRG32 API and ABI overview.
- `docs/score_api.md`: board-local and server score APIs.
- `docs/external_controllers.md`: UART bridge protocol.
- `docs/labs/`: lab handouts, debugging exercises, break/fix assignments.

When editing labs:

- Use concrete student actions.
- Include a visible checkpoint.
- Include a short reflection/debugging question.
- Keep commands copy-pastable.

## VS Code Student Setup

Keep `.vscode` and `PRG32.code-workspace` plug-and-play for students.

Recommended extension list should include:

- Espressif ESP-IDF extension.
- Microsoft C/C++ extension.
- Python extension for the score server.

Tasks should remain simple wrappers around:

- `idf.py set-target esp32c6`
- `idf.py menuconfig`
- `idf.py build`
- `idf.py flash monitor`
- `python3 tools/prg32_score_server/app.py`

Do not hard-code one instructor machine path. Use workspace-relative paths and
standard ESP-IDF configuration variables where possible.

## Score Server Guidelines

The score server lives in `tools/prg32_score_server`.

- Keep it small: Flask + SQLite only.
- API endpoints must stay compatible with board firmware:
  - `GET /api/scores`
  - `POST /api/scores`
- Validate `game`, `player`, and non-negative integer `score`.
- Do not commit generated files such as `.db`, `.db-wal`, `.db-shm`, or
  `__pycache__`.

## Hardware Guidelines

Hardware docs should reinforce the architecture:

```text
USB controller -> USB host bridge -> UART -> ESP32-C6 -> PRG32 bitmask
```

ESP32-C6 is the main RISC-V teaching MCU. Do not document it as a general USB HID
host for arbitrary gamepads. External USB controllers require a bridge board or
PC-side helper that emits the PRG32 UART packet.

## Git and Workspace Rules

- The user may have local changes. Do not revert files you did not change unless
  explicitly asked.
- Treat untracked files as user-owned unless you created them during the current
  task.
- Do not commit or push unless the user asks.
- When committing, use a short, specific message.
- Before committing or finalizing, run at least `git diff --check`.
- If a push is rejected because remote moved, pull/rebase and resolve conflicts
  without force-pushing unless the user explicitly requests a force push.

## Validation Checklist

Before reporting completion, try the relevant subset:

```bash
git diff --check
python3 -m py_compile tools/prg32_score_server/app.py
idf.py build
```

If ESP-IDF is not available, use the first two checks and clearly state that the
firmware build could not be run locally.

For assembly-only example changes, also inspect:

```bash
rg -n "^[0-9]{2,}:|[0-9]{2}f|[0-9]{2}b" examples/games/<name>
```

This catches accidental multi-digit numeric labels in classroom assembly.
