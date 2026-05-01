# Lab 04 - Sound and Timing

## Goal

Add a short beep for an event without making the game feel stuck.

## Steps

1. Call `prg32_audio_beep` during initialization.
2. Add a second beep when a collision occurs.
3. Use a short duration first:

```asm
li a0, 880
li a1, 20
call prg32_audio_beep
```

4. Try `200 ms` and observe how it changes responsiveness.
5. Move the beep so it triggers only once per collision.

## Checkpoint

The game should make a sound on collision and continue moving smoothly.

## Reflection

Why does a blocking audio helper make timing easier to understand but worse for fast games?
