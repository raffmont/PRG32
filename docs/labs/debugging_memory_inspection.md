# Debugging Exercise - Memory Inspection

## Goal

Inspect `.data` variables from an assembly game and connect memory words to on-screen behavior.

## Setup

Use an example game from `examples/games` and set a breakpoint in its update routine.

## Tasks

1. Find the label for the x coordinate, for example `pong_graphics_x`.
2. In GDB, inspect it:

```gdb
x/wx &pong_graphics_x
x/4wx &pong_graphics_x
```

3. Step through the instructions that load, modify, and store the x coordinate.
4. Inspect memory again.
5. Change the value from GDB:

```gdb
set var pong_graphics_x = 120
```

6. Continue execution and observe the display.

## Questions

- Which instruction loads the value from memory?
- Which instruction stores it back?
- What happens if the value is negative?

## Deliverable

Submit the before and after memory values, plus the instruction that caused the change.
