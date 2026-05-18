# Tutorial: Writing a PRG32 Game in C

This tutorial uses the same PRG32 firmware as the assembly labs, but writes the
game logic in C. It is intended for programming classes that want embedded,
visual feedback without starting from hardware driver code.

## Learning Goals

- Split a program into `init`, `update`, and `draw` functions.
- Store game state in variables, arrays, and structs.
- Read the PRG32 input bitmask.
- Draw graphics with the same API used by assembly examples.
- Package a C source file as a `.prg32` cartridge.

## 1. Start From an Existing C Example

Open `examples/games/pong/c/game.c`. It exports three functions:

```c
void pong_c_init(void);
void pong_c_update(void);
void pong_c_draw(void);
```

The resident firmware or cartridge loader calls them in this order:

```text
init once, then update/draw once per frame
```

Checkpoint: identify the variables that store paddle position, ball position,
and ball velocity.

## 2. Read Input

PRG32 buttons are returned as a bitmask:

```c
uint32_t input = prg32_input_read();
if (input & PRG32_BTN_LEFT) {
    paddle_x -= 3;
}
if (input & PRG32_BTN_RIGHT) {
    paddle_x += 3;
}
```

Reflection question: why does this code use `&` instead of `==`?

## 3. Draw a Frame

The graphics API uses integer pixels and RGB565 colors:

```c
prg32_gfx_clear(PRG32_COLOR_BLACK);
prg32_gfx_rect(paddle_x, 188, 64, 8, PRG32_COLOR_WHITE);
prg32_gfx_rect(ball_x, ball_y, 8, 8, PRG32_COLOR_YELLOW);
```

Checkpoint: change the paddle color and ball size, then rebuild.

## 4. Embed a C Example in Firmware

For a temporary lab build, add one C source to `main/CMakeLists.txt`:

```cmake
idf_component_register(
    SRCS
        "main.c"
        "../examples/games/pong/c/game.c"
    REQUIRES prg32
    INCLUDE_DIRS "."
)
```

Then call it from `main/main.c`:

```c
#include "prg32.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void pong_c_init(void);
void pong_c_update(void);
void pong_c_draw(void);

void app_main(void) {
    prg32_init();
    pong_c_init();

    while (1) {
        pong_c_update();
        pong_c_draw();
        prg32_gfx_present();
        vTaskDelay(pdMS_TO_TICKS(33));
    }
}
```

Build for hardware:

```bash
idf.py -B build-esp32c6 -D SDKCONFIG=build-esp32c6/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults set-target esp32c6
idf.py -B build-esp32c6 -D SDKCONFIG=build-esp32c6/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults build
idf.py -B build-esp32c6 -D SDKCONFIG=build-esp32c6/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults flash monitor
```

Or build for QEMU:

```bash
idf.py -B build-qemu -D SDKCONFIG=build-qemu/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu set-target esp32c3
idf.py -B build-qemu -D SDKCONFIG=build-qemu/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu qemu --graphics monitor
```

Restore the default `main/CMakeLists.txt` and `main/main.c` after the lab.

## 5. Build a C Cartridge

After the resident firmware is built, package the same C source as a cartridge:

```bash
python3 tools/prg32_game.py build \
  examples/games/pong/c/game.c \
  --firmware-elf build-esp32c6/PRG32.elf \
  --entry-prefix pong_c \
  --name pong-c \
  --out build-esp32c6/pong-c.prg32
```

Upload to the board:

```bash
python3 tools/prg32_game.py upload build-esp32c6/pong-c.prg32 --url http://192.168.4.1
```

For QEMU, build against `build-qemu/PRG32.elf` and stage with `upload-qemu`.

## 6. Move to Tiles and Platform Games

The platformer C example shows structs, tile flags, and camera follow:

```c
static prg32_platform_actor_t player;

prg32_platform_tile_flags(2, PRG32_TILE_FLAG_SOLID);
prg32_platform_actor_init(&player, 1, 32, 120, 8, 8);
prg32_platform_actor_step(&player, input, 2, -7, 1, 5);
```

Use `examples/games/platformer/c/game.c` when the course reaches:

- structs
- arrays of tile data
- reusable helper functions
- state updated once per frame

## Break and Fix Exercise

Break it:

1. In `pong_c_update`, remove the right boundary clamp.
2. Build and run.
3. Hold RIGHT until the paddle leaves the viewport.

Fix it:

1. Restore the clamp.
2. Explain which condition protects the screen boundary.
3. Add a second check for the ball and explain the difference.

## Trainer Notes

- Use C examples for programming-language concepts.
- Use the matching assembly examples when the same behavior should be inspected
  at register and stack-frame level.
- Keep the runtime, board, QEMU workflow, and cartridge workflow identical
  across both tracks.
