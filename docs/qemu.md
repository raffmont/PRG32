# PRG32 QEMU Screen Emulator

PRG32 can run on a desktop with Espressif QEMU and show the 320x200 game
viewport in a virtual RGB framebuffer window. This is intended for students who
want to compile and test game graphics before flashing real ESP32-C6 hardware.

The QEMU build uses ESP32-C3 as the emulator target because Espressif documents
the maintained RISC-V QEMU graphics path for ESP32-C3. PRG32 student games still
use the same 32-bit RISC-V calling convention and PRG32 ABI, and the physical
hardware build remains ESP32-C6.

The assembly ABI does not change. A game still calls:

- `prg32_gfx_clear`
- `prg32_gfx_pixel`
- `prg32_gfx_rect`
- `prg32_gfx_text8`
- `prg32_gfx_present`

Only the display backend changes.

## Supported Hosts

Espressif provides QEMU RISC-V packages for:

- macOS Intel
- macOS Apple Silicon
- Windows x64
- Linux x86_64
- Linux arm64

Use ESP-IDF 5.3 or newer; the `esp_lcd_qemu_rgb` component declares that
minimum IDF version. Install ESP-IDF first, then install QEMU:

```bash
python $IDF_PATH/tools/idf_tools.py install qemu-riscv32
```

After installing QEMU, reactivate ESP-IDF:

```bash
. $IDF_PATH/export.sh
```

On Windows, use the ESP-IDF PowerShell or Command Prompt shortcut, or run the
matching ESP-IDF export script from your ESP-IDF installation.

## Linux and macOS Runtime Libraries

Linux and macOS hosts also need the native libraries used by the QEMU window.

Ubuntu or Debian:

```bash
sudo apt-get install -y libgcrypt20 libglib2.0-0 libpixman-1-0 libsdl2-2.0-0 libslirp0
```

macOS with Homebrew:

```bash
brew install libgcrypt glib pixman sdl2 libslirp
```

## First QEMU Run

Use a separate build directory so the physical board configuration and the QEMU
configuration do not overwrite each other.

```bash
idf.py -B build-qemu -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu set-target esp32c3
idf.py -B build-qemu -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu qemu --graphics monitor
```

The second command builds the firmware if needed, starts QEMU, opens the virtual
screen window, and attaches the serial monitor.

Shortcut scripts are also provided:

```bash
tools/qemu.sh
```

Windows PowerShell from an ESP-IDF shell:

```powershell
tools/qemu.ps1
```

## VS Code Tasks

Open `PRG32.code-workspace` and run these tasks:

1. `PRG32: qemu set target esp32c3`
2. `PRG32: qemu build`
3. `PRG32: qemu screen`

For debugger exercises, use two terminals:

1. Run `PRG32: qemu debug server`.
2. Run `PRG32: qemu gdb`.

Then set breakpoints in symbols such as `pong_graphics_update` or
`asteroids_graphics_draw`.

## How It Works

`components/prg32/Kconfig` selects one display backend:

- `CONFIG_PRG32_DISPLAY_ILI9341`: physical ILI9341 SPI TFT.
- `CONFIG_PRG32_DISPLAY_QEMU_RGB`: Espressif QEMU virtual RGB panel.

The QEMU build uses `sdkconfig.defaults.qemu`, which enables
`CONFIG_PRG32_DISPLAY_QEMU_RGB`. The component manifest
`components/prg32/idf_component.yml` pulls in `espressif/esp_lcd_qemu_rgb` only
for the ESP32-C3 emulator target.

The physical firmware still uses the ILI9341 backend by default.

## Cartridges in QEMU

QEMU uses the same uploadable `.prg32` game package as the real board, but QEMU
does not emulate the classroom Wi-Fi AP. Stage the cartridge into the emulator
flash image before starting QEMU:

```bash
python3 tools/prg32_game.py build \
  examples/games/asteroids/graphics/game.S \
  --firmware-elf build-qemu/PRG32.elf \
  --entry-prefix asteroids_graphics \
  --name asteroids \
  --out build-qemu/asteroids.prg32

python3 tools/prg32_game.py upload-qemu build-qemu/asteroids.prg32
```

If `build-qemu/qemu_flash.bin` does not exist yet, start QEMU once so ESP-IDF
generates the flash image, quit QEMU, then run `upload-qemu`.

Then run `PRG32: qemu screen`.

## Running an Example Game

Example games stay outside the default app. To test one in QEMU, wire it into
`main/CMakeLists.txt` and `main/main.c` exactly as you would for the real board,
then run:

```bash
idf.py -B build-qemu -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu qemu --graphics monitor
```

The same source can later be built for the physical board:

```bash
idf.py build
idf.py flash monitor
```

## Input in QEMU

QEMU screen emulation validates rendering, frame timing, and most assembly
debugging exercises. The QEMU defaults disable physical GPIO buttons, the buzzer,
and the controller bridge, so desktop QEMU runs will usually show the game at
rest unless the app is extended with a host-side input bridge.

The PRG32 input ABI remains the same:

```text
'U' 'G' <button-mask-low> <button-mask-high>
```

This makes it possible to add a host-side keyboard/gamepad bridge later without
changing student games.

## Troubleshooting

If `idf.py qemu --graphics monitor` says `qemu-system-riscv32` is missing,
install the QEMU tool and reactivate ESP-IDF:

```bash
python $IDF_PATH/tools/idf_tools.py install qemu-riscv32
. $IDF_PATH/export.sh
```

If the virtual screen window does not appear, confirm that `--graphics` is in the
command and that the host SDL2 libraries are installed.

If a real board flash fails or crashes during display initialization, check that
you did not use the `build-qemu` directory. The QEMU build targets ESP32-C3 and
uses a virtual panel that does not exist on real PRG32 hardware.

## References

- [ESP-IDF QEMU Emulator for ESP32-C3](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/api-guides/tools/qemu.html)
- [Espressif `esp_lcd_qemu_rgb` component](https://components.espressif.com/components/espressif/esp_lcd_qemu_rgb)
- [IDF Component Manager manifest reference](https://docs.espressif.com/projects/idf-component-manager/en/latest/reference/manifest_file.html)
