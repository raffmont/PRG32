# PRG32 Example Games

These games are external examples. They are not built by the default firmware
app, because the default app stays a small resident runtime for lessons and
uploadable cartridges.

Every game has assembly and C versions:

```text
examples/games/<game>/
|-- README.md
|-- ascii/game.S
|-- graphics/game.S
`-- c/game.c
```

## Included Games

| Game | ASCII prefix | Graphics prefix | C prefix |
|---|---|---|---|
| `pong` | `pong_ascii` | `pong_graphics` | `pong_c` |
| `breakout` | `breakout_ascii` | `breakout_graphics` | `breakout_c` |
| `space_invaders` | `space_invaders_ascii` | `space_invaders_graphics` | `space_invaders_c` |
| `pacman` | `pacman_ascii` | `pacman_graphics` | `pacman_c` |
| `asteroids` | `asteroids_ascii` | `asteroids_graphics` | `asteroids_c` |
| `tetris` | `tetris_ascii` | `tetris_graphics` | `tetris_c` |
| `platformer` | `platformer_ascii` | `platformer_graphics` | `platformer_c` |

Use the prefix to find the three exported symbols:

```text
<prefix>_init
<prefix>_update
<prefix>_draw
```

For example, `tetris_graphics` exports:

```text
tetris_graphics_init
tetris_graphics_update
tetris_graphics_draw
```

## Choose a Mode

ASCII versions are best for:

- first register tracing exercises
- checking variables through console output
- practicing `prg32_console_write` and `prg32_console_hex32`

Graphics versions are best for:

- viewport drawing
- collision or boundary checks
- QEMU screen testing
- final classroom demos

C versions are best for:

- first C programming labs
- comparing assembly and C implementations
- showing that PRG32 is a small API, not an assembly-only runtime

The same source can be used in two ways:

- embedded in the firmware app for a temporary lab build
- packaged as an uploadable `.prg32` cartridge

## Run Embedded in Firmware

This method is useful when a lab wants students to see how source files enter an
ESP-IDF firmware build. It temporarily replaces the default resident cartridge
loop with direct calls into one example game.

The example below embeds the platformer C version. To use a
different game or mode, replace the path and symbol prefix using the table above.

### 1. Add the Game Source to `main/CMakeLists.txt`

```cmake
idf_component_register(
    SRCS
        "main.c"
        "../examples/games/platformer/c/game.c"
    REQUIRES prg32
    INCLUDE_DIRS "."
)
```

### 2. Temporarily Replace `main/main.c`

```c
#include "prg32.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void platformer_c_init(void);
void platformer_c_update(void);
void platformer_c_draw(void);

void app_main(void) {
    prg32_init();
    platformer_c_init();

    while (1) {
        platformer_c_update();
        platformer_c_draw();
        prg32_gfx_present();
        vTaskDelay(pdMS_TO_TICKS(33));
    }
}
```

For the ASCII version, use:

```c
void tetris_ascii_init(void);
void tetris_ascii_update(void);
void tetris_ascii_draw(void);
```

and call the `tetris_ascii_*` functions in the loop.

For the RISC-V graphics version, use:

```c
void platformer_graphics_init(void);
void platformer_graphics_update(void);
void platformer_graphics_draw(void);
```

and add `../examples/games/platformer/graphics/game.S` to `SRCS`.

### 3. Build for the Physical ESP32-C6 Board

```bash
idf.py -B build-esp32c6 -D SDKCONFIG=build-esp32c6/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults set-target esp32c6
idf.py -B build-esp32c6 -D SDKCONFIG=build-esp32c6/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults build
idf.py -B build-esp32c6 -D SDKCONFIG=build-esp32c6/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults flash monitor
```

### 4. Or Build for QEMU Graphics Testing

```bash
idf.py -B build-qemu -D SDKCONFIG=build-qemu/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu set-target esp32c3
idf.py -B build-qemu -D SDKCONFIG=build-qemu/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu qemu --graphics monitor
```

QEMU shows the graphics viewport in a desktop window. Final input, sound, Wi-Fi,
and display wiring checks still belong on the physical board.

### 5. Restore the Resident Runtime

After the lab, restore `main/CMakeLists.txt` to the default:

```cmake
idf_component_register(
    SRCS "main.c"
    REQUIRES prg32
    INCLUDE_DIRS "."
)
```

Then restore the default `main/main.c` resident firmware loop before committing.

## Run as an Uploadable Cartridge on Hardware

This is the normal PRG32 workflow after the resident firmware has been flashed
once.

### 1. Build and Flash the Resident Firmware

```bash
idf.py -B build-esp32c6 -D SDKCONFIG=build-esp32c6/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults set-target esp32c6
idf.py -B build-esp32c6 -D SDKCONFIG=build-esp32c6/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults build
idf.py -B build-esp32c6 -D SDKCONFIG=build-esp32c6/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults flash monitor
```

The board starts the `PRG32` Wi-Fi access point by default for cartridge uploads.

### 2. Build an ASCII Cartridge

```bash
python3 tools/prg32_game.py build \
  examples/games/tetris/ascii/game.S \
  --firmware-elf build-esp32c6/PRG32.elf \
  --entry-prefix tetris_ascii \
  --name tetris-ascii \
  --out build-esp32c6/tetris-ascii.prg32
```

### 3. Build a Graphics Cartridge

```bash
python3 tools/prg32_game.py build \
  examples/games/tetris/graphics/game.S \
  --firmware-elf build-esp32c6/PRG32.elf \
  --entry-prefix tetris_graphics \
  --name tetris-graphics \
  --out build-esp32c6/tetris-graphics.prg32
```

### 3b. Build a C Cartridge

```bash
python3 tools/prg32_game.py build \
  examples/games/platformer/c/game.c \
  --firmware-elf build-esp32c6/PRG32.elf \
  --entry-prefix platformer_c \
  --name platformer-c \
  --out build-esp32c6/platformer-c.prg32
```

### 4. Upload a Cartridge to the Board

Connect the computer to the `PRG32` Wi-Fi network, then upload:

```bash
python3 tools/prg32_game.py upload \
  build-esp32c6/tetris-graphics.prg32 \
  --url http://192.168.4.1
```

To upload the ASCII version, replace the cartridge path with
`build-esp32c6/tetris-ascii.prg32`. To upload the C version, use
`build-esp32c6/platformer-c.prg32`.

## Run as an Uploadable Cartridge in QEMU

QEMU uses the same `.prg32` cartridge format, but the host tool stages the
cartridge directly into the emulator flash image.

### 1. Build QEMU Firmware

```bash
idf.py -B build-qemu -D SDKCONFIG=build-qemu/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu set-target esp32c3
idf.py -B build-qemu -D SDKCONFIG=build-qemu/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu build
```

### 2. Run QEMU Once to Create the Flash Image

```bash
idf.py -B build-qemu -D SDKCONFIG=build-qemu/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu qemu --graphics monitor
```

Stop QEMU after the first successful launch. This creates
`build-qemu/qemu_flash.bin`.

### 3. Build a QEMU Cartridge

```bash
python3 tools/prg32_game.py build \
  examples/games/tetris/graphics/game.S \
  --firmware-elf build-qemu/PRG32.elf \
  --entry-prefix tetris_graphics \
  --name tetris-graphics \
  --out build-qemu/tetris-graphics.prg32
```

For ASCII mode, use `examples/games/tetris/ascii/game.S`,
`--entry-prefix tetris_ascii`, and an ASCII output file name.

For C mode, use `examples/games/platformer/c/game.c`,
`--entry-prefix platformer_c`, and a C output file name.

### 4. Stage the Cartridge into QEMU Flash

```bash
python3 tools/prg32_game.py upload-qemu \
  build-qemu/tetris-graphics.prg32 \
  --flash build-qemu/qemu_flash.bin
```

### 5. Run QEMU Again

```bash
idf.py -B build-qemu -D SDKCONFIG=build-qemu/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu qemu --graphics monitor
```

The resident firmware loads the staged cartridge from the QEMU flash image.

## Quick Substitution Pattern

To run another game, substitute these three values:

```text
GAME=tetris
MODE=graphics
PREFIX=tetris_graphics
```

Then use:

```text
examples/games/<GAME>/<MODE>/game.S
--entry-prefix <PREFIX>
```

For C examples, use:

```text
examples/games/<GAME>/c/game.c
--entry-prefix <GAME>_c
```

Examples:

- Pong ASCII cartridge: `pong`, `ascii`, `pong_ascii`
- Asteroids graphics cartridge: `asteroids`, `graphics`, `asteroids_graphics`
- Tetris graphics cartridge: `tetris`, `graphics`, `tetris_graphics`
- Platformer C cartridge: `platformer`, `c`, `platformer_c`
