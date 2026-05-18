# Teaching With PRG32

PRG32 can serve two related courses with the same firmware, examples, and tools:

- Computing Architecture / RISC-V assembly
- Introductory or intermediate C programming

The key teaching idea is that both tracks call the same PRG32 runtime API. The
difference is the language used to express game logic.

## Shared Runtime Model

Every example uses three entry points:

```text
<prefix>_init
<prefix>_update
<prefix>_draw
```

This structure works for:

- assembly files such as `examples/games/platformer/graphics/game.S`
- C files such as `examples/games/platformer/c/game.c`
- uploadable `.prg32` cartridges
- temporary embedded firmware builds

Students can therefore compare the same idea in two languages without changing
the board, emulator, display, or cartridge workflow.

## Computing Architecture Track

Use assembly examples when the learning goal is:

- RISC-V calling convention
- register lifetimes
- stack frames and `ra`
- memory variables in `.data`
- ABI boundaries between assembly and C
- debugging with register tracing and memory inspection

Recommended sequence:

1. `docs/tutorial_ascii_game.md`
2. `docs/labs/lab_01_hello_world.md`
3. `docs/labs/lab_02_input.md`
4. `docs/labs/debugging_register_tracing.md`
5. one `examples/games/*/ascii/game.S`
6. one `examples/games/*/graphics/game.S`
7. `examples/games/platformer/graphics/game.S`

Assessment idea: ask students to explain which registers hold arguments for
`prg32_platform_actor_step` and which memory offsets hold actor `x`, `y`, and
state.

## C Programming Track

Use C examples when the learning goal is:

- functions and prototypes
- structs
- arrays and tile maps
- control flow
- simple game loops
- translating requirements into state updates

Recommended sequence:

1. Read `docs/tutorial_c_game.md`.
2. Read `components/prg32/include/prg32.h`.
3. Embed `examples/games/pong/c/game.c` in the firmware.
4. Modify constants and observe movement changes.
5. Package the same source as a `.prg32` cartridge.
6. Move to `examples/features/animated_sprites/c/demo.c`.
7. Finish with `examples/games/platformer/c/game.c`.

Assessment idea: ask students to add a new tile flag to the platformer and
explain how `prg32_platform_actor_t` changes every frame.

## Trainer Workflow

For assembly lessons:

```bash
python3 tools/prg32_game.py build \
  examples/games/platformer/graphics/game.S \
  --firmware-elf build-esp32c6/PRG32.elf \
  --entry-prefix platformer_graphics \
  --name platformer-asm \
  --out build-esp32c6/platformer-asm.prg32
```

For C lessons:

```bash
python3 tools/prg32_game.py build \
  examples/games/platformer/c/game.c \
  --firmware-elf build-esp32c6/PRG32.elf \
  --entry-prefix platformer_c \
  --name platformer-c \
  --out build-esp32c6/platformer-c.prg32
```

Upload either cartridge with:

```bash
python3 tools/prg32_game.py upload build-esp32c6/platformer-c.prg32 --url http://192.168.4.1
```

For QEMU, build against `build-qemu/PRG32.elf` and use `upload-qemu`.

## Comparing Assembly And C

Use the platformer examples as a side-by-side activity:

| Concept | Assembly file | C file |
|---|---|---|
| Tile behavior | `platformer/graphics/game.S` | `platformer/c/game.c` |
| Actor state | `.space 24` actor memory | `prg32_platform_actor_t player` |
| Movement | `call prg32_platform_actor_step` | direct C call |
| Drawing | register arguments | function arguments |
| Camera | `prg32_platform_camera_follow` | same helper |

The shared API keeps the cognitive load on language and architecture, not on
different engines.
