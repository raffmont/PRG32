# PRG32 Labs

These labs are designed for 90 minute classroom blocks. Each lab has a concrete board-visible result and a short reflection task.

Labs can be run on real ESP32-C6 hardware or in QEMU with the virtual PRG32
screen. Use QEMU for early graphics/debugging practice when boards are not
available, then validate final wiring, buttons, and sound on physical hardware.

## Sequence

1. `lab_01_hello_world.md`: build, flash, and change console output.
2. `lab_02_input.md`: read GPIO buttons and controller bits.
3. `lab_03_graphics.md`: draw and move a rectangle.
4. `lab_04_sound.md`: add sound effects and frame timing.
5. `lab_05_scores_and_controllers.md`: submit scores and use the UART controller bridge.

## Debugging

- `debugging_register_tracing.md`
- `debugging_memory_inspection.md`

## Assignments

- `break_fix_assignments.md`

Recommended pattern: students commit the working baseline before each break/fix task, then submit a short note explaining the bug, the evidence, and the fix.

## QEMU Lab Workflow

1. Run `PRG32: qemu set target esp32c3`.
2. Run `PRG32: qemu screen`.
3. For GDB labs, run `PRG32: qemu debug server`, then `PRG32: qemu gdb`.

See `docs/qemu.md` for host setup.
