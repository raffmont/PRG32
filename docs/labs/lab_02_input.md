# Lab 02 - Input Bitmasks

## Goal

Read buttons as a single integer and test bits with RISC-V instructions.

## Steps

1. Open `components/prg32/include/prg32.h`.
2. Find the `PRG32_BTN_*` bit definitions.
3. In an assembly update routine, call:

```asm
call prg32_input_read
mv t0, a0
andi t1, t0, 1
```

4. If `t1` is not zero, move an object left.
5. Repeat for right, up, and down.
6. Print the raw input mask with `prg32_console_hex32`.

## Checkpoint

The object must move only while the matching button is pressed.

This checkpoint needs physical buttons or a UART controller bridge. The QEMU
defaults disable physical input pins, so QEMU is useful for stepping through the
bitmask code in GDB but not for the final button demonstration.

## Debug Question

Why is `andi` enough for `PRG32_BTN_LEFT` but not enough for every possible bitmask test?
