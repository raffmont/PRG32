# PRG32 Teaching Pipeline

This document defines a structured teaching progression using PRG32.

## Learning goals

Students should learn:

- How low-level programs interact with a runtime
- The structure of a game loop
- Memory layout and ABI concepts
- Input/output handling in embedded systems
- Performance vs readability trade-offs

## Module structure

### Lab 1 — Runtime understanding

- Read `main/main.c`
- Identify init/update/draw loop
- Trace frame progression

Deliverable: short report explaining runtime loop

---

### Lab 2 — Minimal cartridge

- Write a simple `.S` file exporting `init`, `update`, `draw`
- Print text using console API

Deliverable: “Hello PRG32” cartridge

---

### Lab 3 — Input handling

- Read controller state
- React to button presses

Deliverable: interactive text demo

---

### Lab 4 — Graphics primitives

- Draw pixels, rectangles, and text

Deliverable: static screen layout

---

### Lab 5 — Movement and animation

- Update state over frames
- Implement motion using input

Deliverable: moving object demo

---

### Lab 6 — Sprites and collision

- Use sprite APIs
- Implement hitbox detection

Deliverable: simple game mechanic

---

### Lab 7 — Full game

- Combine input, graphics, logic
- Package as `.prg32`

Deliverable: complete playable game

---

### Lab 8 — Reflection and optimization

- Analyze performance
- Discuss ABI and memory constraints

Deliverable: technical report

## Assessment strategy

- 40% labs
- 40% final game
- 20% report and reflection

## Gamification ideas

- Scoreboard via `/api/scores`
- Leaderboards per lab
- Bonus challenges (e.g., optimize frame time)
