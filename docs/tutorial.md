# PRG32 Tutorial

This tutorial turns a clean PRG32 firmware checkout into a small assembly game project.

## 1. Install

1. Install VS Code.
2. Install the recommended extensions when opening `PRG32.code-workspace`.
3. Install ESP-IDF through the Espressif VS Code extension.
4. Connect an ESP32-C6 board, or install `qemu-riscv32` for desktop screen testing.

## 2. Build the Base App

Run the VS Code task `PRG32: set target esp32c6`, then run `PRG32: build`.

Flash with `PRG32: flash monitor`. The monitor should show:

```text
PRG32 Hello World
```

To run without hardware, use the QEMU tasks instead:

1. `PRG32: qemu set target esp32c3`
2. `PRG32: qemu build`
3. `PRG32: qemu screen`

The QEMU screen task opens a virtual 320x200 PRG32 display window.

## 3. Read the API

The public ABI is in `components/prg32/include/prg32.h`.

Assembly code uses normal RISC-V calling convention:

- `a0`, `a1`, `a2`, ... hold arguments.
- `a0` holds the return value.
- Save `ra` before calling C helpers.

## 4. Add Input

Call `prg32_input_read` from assembly:

```asm
call prg32_input_read
andi t0, a0, 1      # PRG32_BTN_LEFT
```

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

## 6. Add Sound

Use `prg32_audio_beep(hz, ms)`:

```asm
li a0, 440
li a1, 60
call prg32_audio_beep
```

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
