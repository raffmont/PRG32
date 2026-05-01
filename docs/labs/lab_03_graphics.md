# Lab 03 - Graphics and Frame Updates

## Goal

Draw a moving rectangle in the 320x200 game viewport.

## Steps

1. Clear the frame:

```asm
li a0, 0
call prg32_gfx_clear
```

2. Load `x` and `y` from `.data`.
3. Draw a rectangle:

```asm
mv a0, t0
mv a1, t1
li a2, 24
li a3, 12
li a4, 65535
call prg32_gfx_rect
```

4. Update `x` by a signed velocity each frame.
5. Reverse velocity at the screen edges.
6. Call `prg32_gfx_present` from the main loop or rely on the base app loop.

## Checkpoint

The rectangle bounces horizontally without leaving old pixels behind.

## Reflection

Explain why clearing every frame is simple but not always efficient.
