# Asteroids Example

This directory contains two RISC-V assembly versions of an Asteroids-inspired PRG32 demo:

- `ascii/game.S`: shows ship coordinates and a frame counter through the PRG32 console API.
- `graphics/game.S`: moves a small ship around a 320x200 viewport while a blocky asteroid drifts across the screen.

Controls:

- LEFT / RIGHT: move horizontally.
- UP / DOWN: move vertically.
- A: add to the score counter and play a short beep.

The source is intentionally verbose and uses only simple PRG32 framework calls so students can trace registers, memory variables, and C helper calls in the debugger.
