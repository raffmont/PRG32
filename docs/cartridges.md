# PRG32 Uploadable Game Cartridges

PRG32 can be flashed once as a resident runtime. After that, students can build
small native RISC-V game cartridges and upload them without reflashing the whole
firmware.

The same cartridge format is used on:

- real ESP32-C6 hardware, uploaded over the PRG32 Wi-Fi HTTP API
- QEMU, staged into the emulator flash image

## Mental Model

```text
PRG32 firmware
|-- display, input, audio, score APIs
|-- cartridge loader
|-- executable cartridge RAM
`-- cart0 flash partition

game.prg32
|-- PRG32 cartridge header
|-- linked RV32 code/data payload
`-- optional AUDIO block
```

The firmware exports the PRG32 API addresses and the cartridge RAM address.
`tools/prg32_game.py` links a game against those addresses and creates a
`.prg32` package. The firmware validates the package, persists it in `cart0`,
loads any optional AUDIO block, copies code into executable cartridge RAM, and
calls:

- `<game>_init`
- `<game>_update`
- `<game>_draw`

## Flash Layout

`partitions_prg32.csv` is used by both hardware and QEMU builds:

```text
factory: resident PRG32 firmware
cart0:   active uploaded cartridge
cart1:   reserved for future rollback or classroom experiments
```

This partition table is selected by `sdkconfig.defaults` and
`sdkconfig.defaults.qemu`.

## Hardware Workflow

Flash the resident firmware one time:

```bash
idf.py -B build-esp32c6 -D SDKCONFIG=build-esp32c6/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults set-target esp32c6
idf.py -B build-esp32c6 -D SDKCONFIG=build-esp32c6/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults build
idf.py -B build-esp32c6 -D SDKCONFIG=build-esp32c6/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults flash monitor
```

The default physical build starts a small Wi-Fi access point:

```text
SSID:     PRG32
Password: prg32game
URL:      http://192.168.4.1
```

Build a cartridge from an assembly or C example. This example uses the firmware
ELF from the local build to obtain the runtime addresses:

```bash
python3 tools/prg32_game.py build \
  examples/games/asteroids/graphics/game.S \
  --firmware-elf build-esp32c6/PRG32.elf \
  --entry-prefix asteroids_graphics \
  --name asteroids \
  --out build-esp32c6/asteroids.prg32
```

Upload it to the board:

```bash
python3 tools/prg32_game.py upload build-esp32c6/asteroids.prg32 --url http://192.168.4.1
```

The firmware stores the cartridge in `cart0` and starts running it from the main
loop. The cartridge remains selected after reset.

## Runtime Query Workflow

If the board is already running and the host can reach its HTTP API, the build
tool can query the runtime directly:

```bash
python3 tools/prg32_game.py build \
  examples/games/asteroids/graphics/game.S \
  --runtime-url http://192.168.4.1 \
  --entry-prefix asteroids_graphics \
  --name asteroids \
  --out build-esp32c6/asteroids.prg32
```

Useful runtime endpoint:

```bash
curl http://192.168.4.1/api/runtime
```

## QEMU Workflow

Build the resident QEMU firmware:

```bash
idf.py -B build-qemu -D SDKCONFIG=build-qemu/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu set-target esp32c3
idf.py -B build-qemu -D SDKCONFIG=build-qemu/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu build
```

Build a cartridge against the QEMU firmware ELF:

```bash
python3 tools/prg32_game.py build \
  examples/games/asteroids/graphics/game.S \
  --firmware-elf build-qemu/PRG32.elf \
  --entry-prefix asteroids_graphics \
  --name asteroids \
  --out build-qemu/asteroids.prg32
```

Stage it into the QEMU flash image:

```bash
python3 tools/prg32_game.py upload-qemu \
  build-qemu/asteroids.prg32 \
  --flash build-qemu/qemu_flash.bin
```

If `build-qemu/qemu_flash.bin` is missing, start QEMU once so ESP-IDF creates
the flash image, quit QEMU, and run the staging command again.

Then start QEMU:

```bash
idf.py -B build-qemu -D SDKCONFIG=build-qemu/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu qemu --graphics monitor
```

QEMU does not emulate the classroom Wi-Fi upload path. Instead, it uses the same
cartridge package and writes it into the same `cart0` partition inside
`qemu_flash.bin`.

## HTTP API

### Runtime Information

```http
GET /api/runtime
```

Returns:

- cartridge ABI version
- cartridge RAM load address
- cartridge RAM size
- PRG32 API import addresses
- currently loaded cartridge metadata

### List Games

```http
GET /api/games
```

Returns the current `cart0` cartridge state.

### Upload Game

```http
POST /api/games
Content-Type: application/octet-stream

<game.prg32 bytes>
```

Validates, stores, and starts the cartridge.

### Select Stored Game

```http
POST /api/games/select
```

Reloads the stored `cart0` cartridge.

## Cartridge Assembly Rules

The existing graphics examples already follow the right shape:

- export `<name>_init`, `<name>_update`, and `<name>_draw`
- use normal RV32 calling convention
- save `ra` before calling PRG32 C helpers
- keep stack alignment at 16 bytes around C calls
- keep code/data small enough for the 32 KiB cartridge RAM window

The cartridge linker resolves normal calls such as:

```asm
call prg32_gfx_clear
call prg32_input_read
call prg32_audio_beep
```

## Cartridge C Rules

C examples use the same entry shape and the same PRG32 ABI:

```c
void platformer_c_init(void);
void platformer_c_update(void);
void platformer_c_draw(void);
```

Build them with the same tool. The builder detects `.c` sources and compiles
them as small freestanding C modules:

```bash
python3 tools/prg32_game.py build \
  examples/games/platformer/c/game.c \
  --firmware-elf build-esp32c6/PRG32.elf \
  --entry-prefix platformer_c \
  --name platformer-c \
  --out build-esp32c6/platformer-c.prg32
```

Keep C cartridges small and avoid standard-library calls. Use the helpers in
`prg32.h` for display, input, audio, sprites, playfields, and platform physics.

## Cartridge AUDIO Blocks

Cartridges can include a trailing AUDIO block marked by
`PRG32_CART_FLAG_AUDIO_BLOCK`. This block carries sample descriptors,
instrument descriptors, tracker events, and raw unsigned 8-bit sample bytes.

Pack an AUDIO block:

```bash
python3 tools/wav2prg32sample.py jump.wav --rate 22050 --out build/jump.raw
python3 tools/prg32audio_pack.py audio.json --out build/audio.block
```

Attach it to a cartridge:

```bash
python3 tools/prg32_game.py build \
  examples/games/asteroids/graphics/game.S \
  --firmware-elf build-esp32c6/PRG32.elf \
  --entry-prefix asteroids_graphics \
  --audio-block build/audio.block \
  --name asteroids-audio \
  --out build-esp32c6/asteroids-audio.prg32
```

The firmware loads the AUDIO block before calling `<game>_init`, so a cartridge
can immediately call `prg32_audio_play_sample(0, 255, 1024)` when sample `0` is
defined in the block. Cartridges without AUDIO blocks remain valid.

## Limits

This is intentionally a classroom loader, not a general dynamic linker.

- Cartridges are linked for one PRG32 firmware build.
- If the firmware is rebuilt, rebuild the cartridges.
- Cartridge RAM is 32 KiB.
- AUDIO blocks are stored after the code payload and count against cartridge
  partition size, not cartridge executable RAM.
- Only one active cartridge slot is currently used.
- QEMU staging requires QEMU to be stopped before patching `qemu_flash.bin`.
