# PRG32 Feature Demos

These demos isolate framework rendering features without full game rules. Use
them when teaching one graphics concept at a time.

| Demo | Assembly source | C source | Entry prefixes | Shows |
|---|---|---|---|---|
| Scrolling/parallax | `scrolling_parallax/demo.S` | `scrolling_parallax/c/demo.c` | `scrolling_parallax` / `_c` | scroll and parallax |
| Animated sprites | `animated_sprites/demo.S` | `animated_sprites/c/demo.c` | `animated_sprites` / `_c` | sprite frames |
| Dual playfield | `dual_playfield/demo.S` | `dual_playfield/c/demo.c` | `dual_playfield` / `_c` | background plus foreground |
| Splash screen | `splash_screen/demo.S` | `splash_screen/c/demo.c` | `splash_screen` / `_c` | startup and game title splash |
| Joystick keyboard | - | `keyboard_input/c/demo.c` | `keyboard_input_c` | alphanumeric text input |
| Wi-Fi setup | - | `wifi_setup/c/demo.c` | `wifi_setup_c` | setup mode and AP/STA choice |
| Audio synth | - | `audio_synth/c/demo.c` | `audio_synth_c` | notes and samples |
| Player 2 input | - | `player2_input/c/demo.c` | `player2_input_c` | two-player bitmasks |

Each demo exports:

```text
<prefix>_init
<prefix>_update
<prefix>_draw
```

## Run Embedded in Firmware

Temporarily add one demo source to `main/CMakeLists.txt`:

```cmake
idf_component_register(
    SRCS
        "main.c"
        "../examples/features/scrolling_parallax/demo.S"
    REQUIRES prg32
    INCLUDE_DIRS "."
)
```

For the C version, use the `c/demo.c` source and `_c` entry prefix:

```cmake
idf_component_register(
    SRCS
        "main.c"
        "../examples/features/scrolling_parallax/c/demo.c"
    REQUIRES prg32
    INCLUDE_DIRS "."
)
```

Then call the exported symbols from `main/main.c`:

```c
#include "prg32.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void scrolling_parallax_init(void);
void scrolling_parallax_update(void);
void scrolling_parallax_draw(void);
void scrolling_parallax_c_init(void);
void scrolling_parallax_c_update(void);
void scrolling_parallax_c_draw(void);

void app_main(void) {
    prg32_init();
    scrolling_parallax_init();

    while (1) {
        scrolling_parallax_update();
        scrolling_parallax_draw();
        prg32_gfx_present();
        vTaskDelay(pdMS_TO_TICKS(33));
    }
}
```

Use either the assembly symbols or the C symbols in `app_main`, not both.

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

Restore the default `main/CMakeLists.txt` and `main/main.c` before committing.

## Run as a Cartridge

After building the resident firmware, package a feature demo with its entry
prefix:

```bash
python3 tools/prg32_game.py build \
  examples/features/animated_sprites/demo.S \
  --firmware-elf build-esp32c6/PRG32.elf \
  --entry-prefix animated_sprites \
  --name animated-sprites \
  --out build-esp32c6/animated-sprites.prg32
```

For the C version:

```bash
python3 tools/prg32_game.py build \
  examples/features/animated_sprites/c/demo.c \
  --firmware-elf build-esp32c6/PRG32.elf \
  --entry-prefix animated_sprites_c \
  --name animated-sprites-c \
  --out build-esp32c6/animated-sprites-c.prg32
```

Upload to hardware:

```bash
python3 tools/prg32_game.py upload \
  build-esp32c6/animated-sprites.prg32 \
  --url http://192.168.4.1
```

For QEMU, build against `build-qemu/PRG32.elf` and stage the cartridge:

```bash
python3 tools/prg32_game.py upload-qemu \
  build-qemu/animated-sprites.prg32 \
  --flash build-qemu/qemu_flash.bin
```
