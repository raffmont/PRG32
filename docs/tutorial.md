# PRG32 Tutorial

This tutorial turns a clean PRG32 firmware checkout into a small assembly game
project. It is designed for short lab sessions: build, observe, change one
thing, and explain what changed.

## 1. Install

1. Install VS Code.
2. Install the recommended extensions when opening `PRG32.code-workspace`.
3. Install ESP-IDF through the Espressif VS Code extension.
4. Connect an ESP32-C6 board, or install `qemu-riscv32` for desktop screen testing.

## 2. Build the Base App

Run the VS Code task `PRG32: set target esp32c6`, then run `PRG32: build`.
These tasks use the dedicated `build-esp32c6` directory and
`sdkconfig.defaults`, so they keep the real ILI9341 board build separate from
QEMU.

Flash with `PRG32: flash monitor`. On the physical ESP32-C6 board, the monitor
shows `app_main()` entering before the LCD SPI driver starts. If USB
secondary-console output is visible, the monitor also shows:

```text
PRG32 boot: app_main entered
I (...) prg32_main: starting PRG32 runtime
I (...) prg32_lcd: ILI9341 SPI2 MOSI=7 MISO=2 SCLK=6 CS=10 DC=8 RST=9 BL=5 ...
I (...) prg32_lcd: ILI9341 initialization complete
PRG32 Hello World
```

Checkpoint:

- The firmware builds without errors.
- The monitor logs the configured ILI9341 pins.
- The board shows the PRG32 Hello World message.
- The serial monitor prints boot logs and the app message when USB
  secondary-console output is visible.

To run without hardware, use the QEMU tasks instead:

1. `PRG32: qemu set target esp32c3`
2. `PRG32: qemu build`
3. `PRG32: qemu screen`

The QEMU screen task opens a virtual 320x200 PRG32 display window.

If QEMU is your first target, remember that it does not emulate the physical
GPIO buttons. Use QEMU for graphics, timing, register tracing, and memory
inspection. Use the real ESP32-C6 board for final button, buzzer, Wi-Fi, and LCD
wiring checks.

## 3. Read the API

The public ABI is in `components/prg32/include/prg32.h`.

Assembly code uses normal RISC-V calling convention:

- `a0`, `a1`, `a2`, ... hold arguments.
- `a0` holds the return value.
- Save `ra` before calling C helpers.
- Keep `sp` 16-byte aligned around C calls.
- Treat `a` and `t` registers as temporary across calls.

## 4. Add Input

Call `prg32_input_read` from assembly:

```asm
call prg32_input_read
andi t0, a0, 1      # PRG32_BTN_LEFT
```

Try it:

1. Read the full input mask into `a0`.
2. Copy it into `t0`.
3. Use `andi` to test one button.
4. Change a variable only when that bit is nonzero.

Use `docs/labs/lab_02_input.md` for the full exercise.

## 5. Add Graphics

Use:

```asm
li a0, 0
call prg32_gfx_clear
li a0, 40
li a1, 80
li a2, 24
li a3, 12
li a4, 65535
call prg32_gfx_rect
```

Checkpoint:

- You can clear the screen.
- You can draw one rectangle.
- You can move that rectangle by changing a memory variable.
- You can explain which arguments went into `a0` to `a4`.

## 6. Add Sound

Use `prg32_audio_beep(hz, ms)`:

```asm
li a0, 440
li a1, 60
call prg32_audio_beep
```

For a first game, play a short beep only on an event such as collision, scoring,
or pressing A. Avoid playing a beep every frame.

## 7. Add Scores

For local in-device scores:

```asm
la a0, game_name
la a1, player_name
li a2, 1200
call prg32_score_submit
```

For a classroom server, run `tools/prg32_score_server/app.py` and call `prg32_score_submit_remote` from C glue code or a wrapper.

## 8. Add a Controller Bridge

External controllers send UART packets:

```text
'U' 'G' <button-mask-low> <button-mask-high>
```

See `docs/external_controllers.md`.

## 9. Build a Complete Game

Start from one example in `examples/games`. Wire one `game.S` into `main/CMakeLists.txt`, then call its exported symbols from `main.c`.

For example, `examples/games/pong/graphics/game.S` exports:

- `pong_graphics_init`
- `pong_graphics_update`
- `pong_graphics_draw`

Keep the first version small: one moving object, one collision, one score counter.

You can test the same game on the QEMU screen first, then build and flash the
physical ILI9341 version when it behaves correctly.

Suggested order:

1. Start with `init`, `update`, and `draw` labels that return immediately.
2. Add one `.data` variable for position.
3. Draw one object at that position.
4. Read input and update the position.
5. Add one boundary check.
6. Add one collision or scoring rule.
7. Add sound only after the core loop works.

## 10. Upload a Game Without Reflashing

After the resident firmware has been flashed once, build the game as a cartridge:

```bash
python3 tools/prg32_game.py build \
  examples/games/pong/graphics/game.S \
  --firmware-elf build-esp32c6/PRG32.elf \
  --entry-prefix pong_graphics \
  --name pong \
  --out build-esp32c6/pong.prg32
```

Connect to the `PRG32` Wi-Fi network, then upload:

```bash
python3 tools/prg32_game.py upload build-esp32c6/pong.prg32 --url http://192.168.4.1
```

Use `docs/cartridges.md` for the full workflow.

## 11. Debug Like a Systems Programmer

When a game misbehaves, do not guess first. Trace state:

1. Print or overlay the input mask.
2. Inspect the variable that should change.
3. Check the register holding the current argument.
4. Verify that `ra` and `sp` are restored after a C call.
5. Reduce the game to the smallest failing case.

Useful questions:

- Which register contained the last correct value?
- Which instruction changed it?
- Did a C helper call overwrite a temporary register you expected to keep?
- Did a branch skip the code that updates memory?

The debugging labs in `docs/labs` turn these questions into exercises.

## 12. Explore Rendering Features

Feature demos under `examples/features` isolate advanced graphics ideas:

- scrolling and parallax playfields
- animated sprites
- dual playfields

Use these demos before combining the ideas in a full game. They are intentionally
shorter than game examples, so each register and memory variable is easier to
trace.
