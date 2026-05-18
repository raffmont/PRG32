# PRG32

PRG32 is an educational runtime for RISC-V assembly and C games.

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

# 4) Build and flash physical ESP32-C6 firmware
idf.py -B build-esp32c6 -D SDKCONFIG=build-esp32c6/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults set-target esp32c6
idf.py -B build-esp32c6 -D SDKCONFIG=build-esp32c6/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults flash monitor

# 5) Build QEMU firmware
idf.py -B build-qemu -D SDKCONFIG=build-qemu/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu set-target esp32c3
idf.py -B build-qemu -D SDKCONFIG=build-qemu/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu build

# 6) Run QEMU once (creates build-qemu/qemu_flash.bin)
idf.py -B build-qemu -D SDKCONFIG=build-qemu/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu qemu --graphics monitor
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

## PlatformIO Quick Start

Open the repository root in PlatformIO. The checked-in `platformio.ini` default
environment is `prg32-esp32c6`, which targets the ESP32-C6 DevKitC-1 with
ESP-IDF, reuses the standard `main` component, and applies
`partitions_prg32.csv` plus `sdkconfig.defaults`.

CLI equivalents:

```bash
pio run
pio run -t upload
pio device monitor -b 115200
```

The ESP32-C6 build keeps UART0 as the primary ESP-IDF console and enables
native USB Serial/JTAG as secondary output for PlatformIO Monitor. A healthy
boot logs the configured `prg32_lcd` ILI9341 pins before drawing the splash.

The PlatformIO environment is for the physical ESP32-C6 classroom board. Keep
using the `idf.py` commands in `docs/qemu.md` for QEMU screen builds.

## 🧠 How PRG32 works

- PRG32 is **not** a CPU instruction emulator.
- Code runs natively on ESP32-C6 hardware, or on Espressif QEMU firmware target
  ESP32-C3 for desktop graphics/testing.
- Games are distributed as cartridges (`.prg32`) loaded by the runtime.
- Input supports one or two digital joystick modules through the same PRG32
  bitmask used by QEMU and cartridge tests.
- Audio supports mandatory mono I2S output and optional stereo PRG32 Audio Plus
  using MAX98357A amplifier breakouts.

Flow:

```text
.S source -> riscv toolchain -> .prg32 cartridge -> PRG32 runtime -> init/update/draw loop
```

## ❗ Troubleshooting

- `idf.py: command not found`: ESP-IDF is not sourced. Run
  `. $HOME/esp-idf/export.sh`.
- Black display on real ESP32-C6 hardware: rebuild with the physical build
  directory, not the QEMU one:
  `idf.py -B build-esp32c6 -D SDKCONFIG=build-esp32c6/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults set-target esp32c6`
  then
  `idf.py -B build-esp32c6 -D SDKCONFIG=build-esp32c6/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults flash monitor`.
  The monitor should log `prg32_lcd` with the configured ILI9341 pins.
- PlatformIO Monitor shows only ROM boot text or only its header: the firmware
  was probably built with an older generated `sdkconfig.prg32-esp32c6`, or the
  USB monitor missed secondary-console output during reset. Delete that
  generated file once, rebuild, upload, open Monitor again, and press RESET/EN
  on the board. A healthy app boot logs `PRG32 boot: app_main entered` and the
  configured `prg32_lcd` pins.
- PlatformIO says it cannot exclusively lock `/dev/cu.usbmodem...`: close every
  other Serial Monitor, ESP-IDF Monitor, Arduino Serial Monitor, and terminal
  using that port, then start only one PlatformIO Monitor.
- QEMU runs but the game does not move: QEMU defaults disable physical GPIO
  buttons. Use a UART bridge packet source, or debug logic with the overlay/GDB.
- Cartridge upload fails: `build-qemu/qemu_flash.bin` is missing/invalid, or the
  cartridge is too large. Run QEMU once, then rerun `upload-qemu`.
- `riscv32-esp-elf-gcc` missing: re-run `./install.sh esp32c3,esp32c6` and
  source the ESP-IDF export script.
- Partition mismatch errors: run `tools/prg32_game.py doctor` and verify
  `partitions_prg32.csv` plus the selected cartridge slot.

## Learning Path

1. Build and flash the resident firmware.
2. Run the Hello World app on the serial monitor and display.
3. Read `docs/tutorial.md` for assembly or `docs/tutorial_c_game.md` for C.
4. Complete the labs in `docs/labs`.
5. Modify one example game under `examples/games`.
6. Try a focused rendering demo under `examples/features`.
7. Package it as a `.prg32` cartridge and upload it over Wi-Fi or stage it into
   the QEMU flash image.

The intended student rhythm is small and visible: change one thing, run it,
observe the result, and write down what changed.

## Feature Demos

The `examples/features` directory contains focused demos for framework features:

- scrolling and parallax playfields
- animated sprites
- dual playfield rendering
- startup and game title splash screens
- joystick-driven on-screen keyboard input
- Wi-Fi setup mode
- audio synthesis and sample playback
- player 2 input

Use these before building a full game when the lesson is about one graphics
technique rather than game rules.

## Audio

PRG32 supports retro-style digital audio through I2S.

The default configuration uses one MAX98357A I2S DAC/amplifier breakout for
mono output. Optional stereo output, called PRG32 Audio Plus, uses two
MAX98357A breakouts on the same I2S bus: one configured for the left channel
and one configured for the right channel.

Start with:

- `examples/audio_mono_beep`
- `examples/audio_mono_sample`
- `examples/audio_mono_tracker`
- `examples/audio_stereo_pan_test`
- `examples/audio_stereo_music`

See [docs/audio.md](docs/audio.md) for wiring, API, cartridge AUDIO blocks, and
troubleshooting.

The resident firmware also shows a logo splash screen on boot. Physical
ESP32-C6 builds then enter Wi-Fi setup mode so students can choose the PRG32
upload access point or connect the board to an existing network. When the audio
configuration is usable for the current board, the splash plays a short welcome
sound; otherwise it falls back to the passive buzzer when configured.

## Example Games

The `examples/games` directory includes ASCII assembly, graphics assembly, and
C versions of:

- `pong`
- `breakout`
- `space_invaders`
- `pacman`
- `asteroids`
- `tetris`
- `platformer`

See [examples/games/README.md](examples/games/README.md) for step-by-step
instructions to run each game embedded in firmware or as an uploadable
cartridge on hardware and QEMU.

## Teaching Tracks

PRG32 supports two classroom tracks over the same runtime:

- Computing Architecture: RISC-V assembly examples under `examples/games/*/ascii`
  and `examples/games/*/graphics`.
- C Programming: C examples under `examples/games/*/c` and
  `examples/features/*/c`.

See [docs/teaching_with_prg32.md](docs/teaching_with_prg32.md) for trainer notes,
lab sequencing, and how to compare assembly and C versions of the same example.
The C programming tutorial is [docs/tutorial_c_game.md](docs/tutorial_c_game.md).

## Runtime APIs

- Runtime/diagnostics: `GET /api/runtime`
- Games: `GET /api/games`, `POST /api/games`, `POST /api/games/select`
- Optional scores: `GET /api/scores`, `POST /api/scores`

`/api/runtime` includes firmware version, cartridge state, frame count, and last input state.

## Asset Tools

- `tools/prg32_image_convert.py`: convert PNG/JPEG/GIF assets to C or assembly.
- `tools/prg32_image_prepare.py`: interactive terminal preparation helper.
- `tools/prg32_audio_convert.py`: convert WAV samples or MIDI tracks.
- `tools/wav2prg32sample.py`: convert WAV to unsigned 8-bit PRG32 sample data.
- `tools/midi2prg32audio.py`: convert simple MIDI notes to tracker JSON.
- `tools/prg32audio_pack.py`: pack samples, instruments, and tracks into an
  AUDIO block for `.prg32` cartridges.

See [docs/assets.md](docs/assets.md).

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

For GitHub Actions and machines without ESP-IDF, the repository also provides a
host-only smoke test:

```bash
bash scripts/ci_smoke_test.sh
```

That check runs Python syntax checks, unit tests, whitespace validation, and the
cartridge tool doctor in host-only mode.

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
- `examples/games`: assembly and C game demos
- `examples/features`: focused firmware feature demos
- `tools/prg32_game.py`: cartridge tooling
- `tools/prg32_image_convert.py`: image and animation converter
- `tools/prg32_audio_convert.py`: sample and MIDI converter
- `tools/prg32audio_pack.py`: AUDIO block packer
- `docs`: tutorials, labs, API docs
- `.github/workflows/ci.yml`: GitHub Actions smoke and firmware build workflow
- `tests`: host-side unit tests for tooling and documentation hygiene

## Repository Structure (Academic View)

- `components/prg32`: framework API/ABI and hardware abstraction layer
- `main`: reference runtime firmware app (minimal and teachable baseline)
- `examples/games`: course demos in RISC-V assembly and C
- `examples/features`: focused rendering demos in RISC-V assembly and C
- `docs/labs`: lab handouts, debugging assignments, and assessment-friendly exercises
- `tools`: reproducible developer tooling (cartridge builder, QEMU scripts, score server)
- `hardware`: architecture notes and board integration scaffold

## Contributors

- Raffaele Montella - UniParthenope - academic supervisor / project lead
- Ivan Cafiero - UniParthenope - Computer Science student

See [CONTRIBUTORS.md](CONTRIBUTORS.md) for contributor metadata suitable for
academic submissions.

## Citation

For reports, theses, or coursework submissions, use the citation metadata in
[CITATION.cff](CITATION.cff).
