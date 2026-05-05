# PRG32

**PRG32** is an educational runtime for building small RISC-V assembly games on ESP32-class hardware and in Espressif QEMU.

It is designed for computer architecture, embedded systems, and systems programming courses where students move from bare assembly routines to a complete interactive runtime.

PRG32 is **not** a CPU emulator. Student code is compiled with the RISC-V toolchain and executed by the PRG32 runtime.

```text
.S source -> RISC-V toolchain -> .prg32 cartridge -> PRG32 runtime -> init/update/draw loop
```

## What PRG32 provides

- A compact game runtime with `init`, `update`, and `draw` entry points
- A `.prg32` cartridge format for packaging RISC-V assembly games
- Graphics, text, tile, sprite, input, diagnostic, and optional score APIs
- A QEMU workflow for desktop testing before hardware deployment
- An ESP32-C6 hardware path for real embedded execution
- A teaching scaffold for labs, exercises, grading, and project work

## Target audience

- Early undergraduate programming courses
- Computer architecture laboratories
- Embedded systems courses
- Creative coding assignments using low-level programming
- Project-based learning and gamified assessment

## Repository map

| Path | Purpose |
|---|---|
| `components/prg32` | Runtime implementation and public API |
| `components/prg32/include/prg32.h` | Public runtime ABI |
| `main` | Reference firmware application |
| `examples/games` | Assembly game examples |
| `tools/prg32_game.py` | Cartridge builder, uploader, and doctor utility |
| `scripts` | Smoke tests and QEMU helpers |
| `docs` | Tutorials, labs, API notes, teaching material |
| `hardware` | Hardware integration notes |

## Quick start: QEMU path

Install ESP-IDF v5.3 with ESP32-C3 and ESP32-C6 support, source the ESP-IDF environment, then build the QEMU target:

```bash
idf.py -B build-qemu -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu set-target esp32c3
idf.py -B build-qemu -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu build
```

Run QEMU once to create the flash image:

```bash
idf.py -B build-qemu -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu qemu --graphics monitor
```

Build and stage a demo cartridge:

```bash
python3 tools/prg32_game.py build examples/games/asteroids/graphics/game.S \
  --firmware-elf build-qemu/PRG32.elf \
  --entry-prefix asteroids_graphics \
  --name asteroids \
  --out build-qemu/asteroids.prg32

python3 tools/prg32_game.py upload-qemu build-qemu/asteroids.prg32 \
  --flash build-qemu/qemu_flash.bin
```

Use the one-command demo workflow after the first setup:

```bash
./scripts/run_qemu_demo.sh
```

## Smoke test

Run before submitting changes:

```bash
./scripts/smoke_test.sh
```

Expected result:

```text
=== SMOKE TEST PASSED ===
```

## Developer tools

```bash
python3 tools/prg32_game.py doctor
```

Use the doctor command to validate toolchain availability, partition assumptions, and cartridge workflow prerequisites.

## Runtime API overview

The public runtime API is declared in `components/prg32/include/prg32.h`.

Main API groups:

- Runtime and diagnostics
- Controller input
- Cartridge lifecycle
- Console output
- Graphics primitives
- Tiles and sprites
- Optional local and remote score submission

## HTTP diagnostic endpoints

| Endpoint | Purpose |
|---|---|
| `GET /api/runtime` | Runtime status, firmware version, cartridge state, frame count, input state |
| `GET /api/games` | List available games |
| `POST /api/games` | Upload or install game cartridge |
| `POST /api/games/select` | Select staged cartridge |
| `GET /api/scores` | Read optional scores |
| `POST /api/scores` | Submit optional score |

## Teaching model

A complete teaching progression is available in `docs/teaching_pipeline.md`.

Recommended progression:

1. Understand the runtime loop
2. Build a minimal cartridge
3. Read controller input
4. Draw text and pixels
5. Add sprites and collision
6. Package, test, and present a complete game
7. Reflect on ABI, memory, performance, and embedded constraints

## Troubleshooting

| Problem | Likely cause | Fix |
|---|---|---|
| `idf.py: command not found` | ESP-IDF environment not sourced | Source ESP-IDF again |
| `riscv32-esp-elf-gcc` missing | Toolchain unavailable | Re-run ESP-IDF install and source the environment |
| QEMU runs but game does not move | GPIO buttons are unavailable in QEMU | Use UART/input bridge, debug overlay, or GDB validation |
| Cartridge upload fails | Missing flash image or oversized cartridge | Run QEMU once, rerun upload, check cartridge size |
| Partition mismatch | Wrong slot or partition layout | Run `tools/prg32_game.py doctor` |

## Contributing

See `CONTRIBUTING.md` for workflow and coding expectations.

## Academic use and citation

Use `CITATION.cff` for reports, theses, and coursework. Contributors are listed in `CONTRIBUTORS.md`.

## License

PRG32 is distributed under the MIT License. See `LICENSE` for details.
