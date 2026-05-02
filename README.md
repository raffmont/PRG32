# PRG32

PRG32 is an educational runtime for RISC-V assembly games.

## Academic Profile

- Project domain: Embedded Systems and Computer Architecture Education
- Platform focus: ESP32-C6 (hardware) and ESP32-C3 QEMU path (desktop emulation)
- Course style: first-year/early undergraduate assembly and systems labs
- Academic supervisor / project lead: Raffaele Montella - UniParthenope
- Contributor (student): Ivan Cafiero - UniParthenope - Computer Science student

## 🚀 Quick Start (macOS)

```bash
# 1) Dependencies
brew install git cmake ninja dfu-util ccache libusb python

# 2) ESP-IDF
cd $HOME
git clone -b v5.3 --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32c3,esp32c6
. ./export.sh

# 3) Project
cd /path/to/PRG32

# 4) Build QEMU firmware
idf.py -B build-qemu -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu set-target esp32c3
idf.py -B build-qemu -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu build

# 5) Run QEMU once (creates build-qemu/qemu_flash.bin)
idf.py -B build-qemu -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu qemu --graphics monitor
```

Open a second terminal (source ESP-IDF again), then stage a demo cartridge:

```bash
cd /path/to/PRG32
. $HOME/esp-idf/export.sh
python3 tools/prg32_game.py build \
  examples/games/asteroids/graphics/game.S \
  --firmware-elf build-qemu/PRG32.elf \
  --entry-prefix asteroids_graphics \
  --name asteroids \
  --out build-qemu/asteroids.prg32
python3 tools/prg32_game.py upload-qemu build-qemu/asteroids.prg32 --flash build-qemu/qemu_flash.bin
```

Run everything with one command next time:

```bash
./scripts/run_qemu_demo.sh
```

## 🍏 macOS Setup (Tested on Apple Silicon)

```bash
brew install git cmake ninja dfu-util ccache libusb python
cd $HOME
git clone -b v5.3 --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32c3,esp32c6
. ./export.sh
```

Quality-of-life alias:

```bash
alias get_idf=". $HOME/esp-idf/export.sh"
```

Common pitfalls:

- New shell opened: run `get_idf` again.
- `idf.py` missing: ESP-IDF not sourced.
- `riscv32-esp-elf-gcc` missing: ESP-IDF toolchain not installed/sourced.

## 🧠 How PRG32 works

- PRG32 is **not** a CPU instruction emulator.
- Code runs natively on ESP32-C6 hardware, or on Espressif QEMU firmware target (ESP32-C3) for desktop graphics/testing.
- Games are distributed as cartridges (`.prg32`) loaded by the runtime.

Flow:

```text
.S source -> riscv toolchain -> .prg32 cartridge -> PRG32 runtime -> init/update/draw loop
```

## ❗ Troubleshooting

| Problem | Cause | Fix |
|---|---|---|
| `idf.py: command not found` | ESP-IDF environment not sourced | Run `. $HOME/esp-idf/export.sh` |
| QEMU runs but game does not move | QEMU defaults disable physical GPIO buttons; no UART bridge input | Use a UART bridge packet source, or verify logic with debug overlay/GDB |
| Cartridge upload fails | Missing/invalid `build-qemu/qemu_flash.bin` or oversized cartridge | Run QEMU once to create flash file, then rerun `upload-qemu`; check cartridge size |
| `riscv32-esp-elf-gcc` missing | Toolchain not in PATH | Re-run `./install.sh esp32c3,esp32c6` and source export script |
| Partition mismatch errors | Cartridge staged using wrong partition layout/slot | Use `tools/prg32_game.py doctor` and verify `partitions_prg32.csv` + slot |

## Runtime APIs

- Runtime/diagnostics: `GET /api/runtime`
- Games: `GET /api/games`, `POST /api/games`, `POST /api/games/select`
- Optional scores: `GET /api/scores`, `POST /api/scores`

`/api/runtime` includes firmware version, cartridge state, frame count, and last input state.

## Developer Tools

- Doctor check:

```bash
python3 tools/prg32_game.py doctor
```

- One-shot QEMU demo:

```bash
./scripts/run_qemu_demo.sh
```

## 🧪 Smoke Test

This script verifies the basic PRG32 workflow end to end: environment setup,
firmware build, cartridge build, and QEMU staging.

```bash
./scripts/smoke_test.sh
```

Expected result:

- all steps print `[OK]`
- warnings may appear but are non-blocking
- final output shows `=== SMOKE TEST PASSED ===`

## Screenshots

Place images under `docs/images/`.

Suggested files:

- `docs/images/prg32-qemu-game.png`
- `docs/images/prg32-upload-success.png`
- `docs/images/prg32-runtime-api.png`

See `docs/images/README.md` for capture instructions.

## Repo Map

- `components/prg32`: runtime implementation
- `main`: default firmware app
- `examples/games`: assembly demos
- `tools/prg32_game.py`: cartridge tooling
- `docs`: tutorials, labs, API docs

## Repository Structure (Academic View)

- `components/prg32`: framework API/ABI and hardware abstraction layer
- `main`: reference runtime firmware app (minimal and teachable baseline)
- `examples/games`: course demos in RISC-V assembly (`ascii` + `graphics`)
- `docs/labs`: lab handouts, debugging assignments, and assessment-friendly exercises
- `tools`: reproducible developer tooling (cartridge builder, QEMU scripts, score server)
- `hardware`: architecture notes and board integration scaffold

## Contributors

- Raffaele Montella - UniParthenope - academic supervisor / project lead
- Ivan Cafiero - UniParthenope - Computer Science student

See [CONTRIBUTORS.md](/Users/ivan/Desktop/PRG32/CONTRIBUTORS.md) for contributor metadata suitable for academic submissions.

## Citation

For reports, theses, or coursework submissions, use the citation metadata in
[CITATION.cff](/Users/ivan/Desktop/PRG32/CITATION.cff).
