# PRG32 Example Games

These games are external examples. They are not built by the default firmware app.

## Included Games

- `pong`
- `breakout`
- `space_invaders`
- `pacman`
- `asteroids`

Each game has:

- `ascii/game.S`
- `graphics/game.S`

## Using an Example in a Lab

Copy or reference one game source from `examples/games` in `main/CMakeLists.txt`, then provide a small C wrapper or select the symbols required by the lab.

Minimal CMake experiment:

```cmake
idf_component_register(
    SRCS
        "main.c"
        "../examples/games/pong/graphics/game.S"
    REQUIRES prg32
    INCLUDE_DIRS "."
)
```

Then call the exported example symbols from `main.c`:

```c
void pong_graphics_init(void);
void pong_graphics_update(void);
void pong_graphics_draw(void);
```

The default PRG32 app is intentionally a clean Hello World so students learn how source files enter the build.
