# Break the Game and Fix It

Each assignment starts from a working example game. Students must introduce or diagnose the bug, gather evidence, and repair it.

Most graphics and register/memory bugs can be reproduced in QEMU with
`PRG32: qemu screen` or the QEMU GDB tasks. Wiring, button, buzzer, and
ILI9341-specific problems still require the physical board.

## Assignment 1 - Lost Return Address

Break it: remove the `sw ra, 12(sp)` / `lw ra, 12(sp)` pair around a function that calls a PRG32 helper.

Symptoms: the program returns to the wrong place or crashes after a C call.

Fix it: restore the stack frame and explain why `call` overwrites `ra`.

## Assignment 2 - Inverted Button Logic

Break it: change one `andi` mask so LEFT tests the RIGHT bit.

Symptoms: a button moves the object in the wrong direction.

Fix it: compare the mask with `PRG32_BTN_*` in `prg32.h`.

## Assignment 3 - Rectangle Never Clears

Break it: remove `prg32_gfx_clear` before drawing.

Symptoms: the moving object leaves trails.

Fix it: clear the frame or redraw the previous rectangle with the background color.

## Assignment 4 - Out-of-Bounds Motion

Break it: remove the screen-edge clamp or velocity reversal.

Symptoms: the object disappears.

Fix it: use memory inspection to find the coordinate and add bounds checks.

## Assignment 5 - Bad UART Packet

Break it: send `U X lo hi` instead of `U G lo hi` from the controller bridge.

Symptoms: external controller input stops updating while GPIO buttons still work.

Fix it: restore the packet header and explain how the parser resynchronizes.

## Assignment 6 - Score JSON Error

Break it: POST a score without `player` or with a negative score.

Symptoms: `/api/scores` rejects the record.

Fix it: correct the JSON and identify which validation branch returned the error.

## Assignment 7 - Wrong Emulator Build

Break it: try to use the QEMU build directory as if it were the physical board
firmware.

Symptoms: the target is ESP32-C3 instead of ESP32-C6, or the firmware stops
during display initialization because the QEMU RGB panel is not present on real
hardware.

Fix it: rebuild with the default ILI9341 backend for the board, or use the QEMU
build only with `idf.py qemu --graphics monitor`.

## Assignment 8 - Cartridge Linked for the Wrong Firmware

Break it: build a `.prg32` cartridge using an old `build/PRG32.elf`, then flash a
new resident firmware and upload the old cartridge.

Symptoms: `/api/games` rejects the upload with a runtime-address error, or the
game does not start.

Fix it: rebuild the cartridge against the current firmware ELF or use
`--runtime-url http://192.168.4.1` so the tool queries the running board.
