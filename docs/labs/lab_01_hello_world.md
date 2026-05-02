# Lab 01 - Hello World on PRG32

## Goal

Build, flash, and edit the minimal PRG32 app.

## Steps

1. Open `PRG32.code-workspace`.
2. Run task `PRG32: set target esp32c6`.
3. Run task `PRG32: build`.
4. Run task `PRG32: flash monitor`.
5. Confirm the monitor prints `PRG32 Hello World`.
6. Edit `main/main.c` and change the message to include your name.
7. Build and flash again.

## Checkpoint

Show the instructor the serial monitor and, if LCD mode is enabled, the display.

## QEMU Option

If no board is available, run:

1. `PRG32: qemu set target esp32c3`
2. `PRG32: qemu screen`

The QEMU monitor should print `PRG32 Hello World`, and the virtual screen should
show the framework console text.

## Reflection

Which C function actually writes one character to UART and LCD? Find it and write the file name.
