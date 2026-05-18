# Tutorial: Writing a Graphic Game in RISC-V Assembly

The graphics mode exposes a 320x200 retro viewport. The same assembly calls work
on the physical ILI9341 display and on the QEMU virtual RGB screen.

## Learning Goals

- Draw pixels, rectangles, and text.
- Keep game state in memory.
- Split work into `init`, `update`, and `draw`.
- Use QEMU for quick visual testing.
- Add a simple collision rule.

## Useful Calls

- `prg32_gfx_clear(color)`
- `prg32_gfx_pixel(x, y, color)`
- `prg32_gfx_rect(x, y, w, h, color)`
- `prg32_gfx_text8(x, y, text, fg, bg)`
- `prg32_gfx_present()`
- `prg32_sprite_hitbox(ax, ay, aw, ah, bx, by, bw, bh)`
- `prg32_sprite_anim_frame(now_ms, frame_count, frame_ms)`
- `prg32_sprite_draw_frame(x, y, w, h, frames, frame, transparent)`
- `prg32_playfield_scroll(layer, x, y)`
- `prg32_playfield_parallax(layer, x_q8, y_q8)`
- `prg32_playfield_draw_dual()`

The framework internally tracks dirty rectangles, so students can draw only
changed objects when they are ready for optimization exercises.

## 1. Start With a Blank Frame

Create `examples/games/my_graphics_game/graphics/game.S`:

```asm
.option norelax
.section .text

.global my_graphics_init
.global my_graphics_update
.global my_graphics_draw

my_graphics_init:
    ret

my_graphics_update:
    ret

my_graphics_draw:
    addi sp, sp, -16
    sw ra, 12(sp)
    li a0, 0                # black
    call prg32_gfx_clear
    lw ra, 12(sp)
    addi sp, sp, 16
    ret
```

Checkpoint: the screen clears without crashing.

## 2. Draw One Rectangle

Add a visible object:

```asm
    li a0, 40               # x
    li a1, 80               # y
    li a2, 24               # width
    li a3, 12               # height
    li a4, 0xffff           # white
    call prg32_gfx_rect
```

Debug question: which register controls the color?

## 3. Store Position in Memory

Move the rectangle coordinates into `.data`:

```asm
.section .data
player_x:
    .word 40
player_y:
    .word 80
```

Load the coordinates before drawing:

```asm
    la t0, player_x
    lw a0, 0(t0)
    la t0, player_y
    lw a1, 0(t0)
```

Checkpoint: changing the `.word` values changes the starting position.

## 4. Add Input

Read the input mask in `update` and change `player_x`:

```asm
my_graphics_update:
    addi sp, sp, -16
    sw ra, 12(sp)
    call prg32_input_read
    mv t2, a0

    la t0, player_x
    lw t1, 0(t0)

    andi t3, t2, 1          # PRG32_BTN_LEFT
    beqz t3, .Lright
    addi t1, t1, -2

.Lright:
    andi t3, t2, 2          # PRG32_BTN_RIGHT
    beqz t3, .Lstore
    addi t1, t1, 2

.Lstore:
    sw t1, 0(t0)
    lw ra, 12(sp)
    addi sp, sp, 16
    ret
```

If the object moves too fast, reduce the step. If it leaves the screen, add
boundary checks.

## 5. Add a Boundary Check

Clamp the X coordinate to the visible area:

```asm
    blt t1, zero, .Lmin_x
    li t4, 296              # 320 - rectangle width
    bgt t1, t4, .Lmax_x
    j .Lstore

.Lmin_x:
    li t1, 0
    j .Lstore

.Lmax_x:
    li t1, 296
```

Checkpoint: holding LEFT or RIGHT never moves the rectangle off-screen.

## 6. Use the Debug Overlay

During debugging, draw state on top of the frame:

```c
prg32_debug_overlay_draw(1, x, y, input_mask, frame_counter);
```

In assembly-only labs, ask the resident C loop to enable the overlay with
`PRG32_DEBUG`, or print selected variables through console helpers.

## 7. Test on QEMU

For a desktop graphics test, run:

```bash
idf.py -B build-qemu -D SDKCONFIG=build-qemu/sdkconfig -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu qemu --graphics monitor
```

QEMU is best for:

- checking layout and movement
- stepping through code with GDB
- inspecting memory variables
- staging `.prg32` cartridges into a flash image

Physical hardware is still required for LCD wiring, real buttons, buzzer output,
and Wi-Fi upload validation.

## 8. Try Framework Feature Demos

Before building a full scrolling game, run the focused feature demos:

- `examples/features/scrolling_parallax/demo.S`
- `examples/features/animated_sprites/demo.S`
- `examples/features/dual_playfield/demo.S`

C versions are available under each demo's `c/demo.c` path for programming
classes that want the same rendering concept without register-level tracing.

Each demo exports `init`, `update`, and `draw` symbols with its directory prefix.
Build them exactly like a game example, either embedded in firmware or as a
`.prg32` cartridge.

## Break and Fix Exercise

Break it:

1. Change the rectangle width argument from `a2` to `a1`.
2. Build and run.
3. Observe the visual error.

Fix it:

1. Restore the argument order.
2. Rebuild.
3. Explain why a wrong register produced a wrong rectangle.

## Reflection

- Which function clears the frame?
- Which memory labels store player state?
- Which branch prevents off-screen movement?
- Which tests should run before flashing hardware?
