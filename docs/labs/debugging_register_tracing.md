# Debugging Exercise - Register Tracing

## Goal

Use GDB to watch RISC-V register values across calls from assembly into the PRG32 C framework.

## Setup

1. Build with debug symbols.
2. Start the VS Code configuration `PRG32: ESP-IDF debug`.
3. Set a breakpoint on an assembly update routine, such as `pong_graphics_update`.

## Tasks

1. Step until just before `call prg32_input_read`.
2. Record `ra`, `sp`, `a0`, `t0`, and `t1`.
3. Step over the call.
4. Record `a0`.
5. Press one button and repeat.
6. Explain which register contains the PRG32 input mask.

## Questions

- Why must assembly save `ra` before calling C helpers?
- Which registers are caller-saved in this example?
- What changes when a button is held?

## Deliverable

Submit a small table with one row before the call and one row after the call.
