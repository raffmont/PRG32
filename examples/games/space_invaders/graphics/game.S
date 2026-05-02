/*
 * SPACE INVADERS GRAPHICS for PRG32.
 * This file is intentionally verbose: every instruction is commented so
 * first-year students can connect RISC-V assembly to visible hardware effects.
 */
.option norelax
.section .data
invaders_graphics_x:      .word 40          /* Invader formation x. */
invaders_graphics_y:      .word 20          /* Invader formation y. */
invaders_graphics_dx:     .word 2           /* Formation horizontal velocity. */
invaders_graphics_cannon_x:.word 120        /* Player cannon x coordinate. */
invaders_graphics_cannon_dx:.word 2         /* Cannon auto patrol speed. */
invaders_graphics_shot_x: .word 134         /* Shot x coordinate. */
invaders_graphics_shot_y: .word 170         /* Shot y coordinate. */
invaders_graphics_score:  .word 0           /* Simple score/counter. */

.section .rodata
invaders_graphics_title: .asciz "SPACE INVADERS GRAPHICS\n"
invaders_graphics_msg:   .asciz "PRG32 RISC-V ASM DEMO\n"

.section .text
.global invaders_graphics_init
.global invaders_graphics_update
.global invaders_graphics_draw

invaders_graphics_init:
    addi sp, sp, -16              /* Build a tiny stack frame before calling C. */
    sw ra, 12(sp)                 /* Save return address because call overwrites ra. */
    call prg32_console_clear      /* Clear UART/LCD text console. */
    la a0, invaders_graphics_title    /* Put address of title string into argument a0. */
    call prg32_console_write      /* Print title through framework console. */
    la a0, invaders_graphics_msg      /* Load second message pointer into a0. */
    call prg32_console_write      /* Print explanatory message. */
    li a0, 120                    /* Choose a short beep frequency/duration argument. */
    li a1, 30                     /* 30 milliseconds: audible but not annoying. */
    call prg32_audio_beep         /* Call C PWM audio helper. */
    lw ra, 12(sp)                 /* Restore return address. */
    addi sp, sp, 16               /* Destroy stack frame. */
    ret                           /* Return to the C game loop. */

invaders_graphics_update:
    addi sp, sp, -16              /* Save caller state before update work. */
    sw ra, 12(sp)                 /* Preserve return address. */

    /* Invader formation moves side-to-side and steps down on wall bounce. */
    la t1, invaders_graphics_x
    lw t2, 0(t1)
    la t3, invaders_graphics_dx
    lw t4, 0(t3)
    add t2, t2, t4
    li t5, 8
    blt t2, t5, 1f
    li t5, 260                     /* 3 invaders width envelope. */
    ble t2, t5, 2f
    li t2, 260
    neg t4, t4
    la t6, invaders_graphics_y
    lw t0, 0(t6)
    addi t0, t0, 8                 /* Descend one row on bounce. */
    li t5, 96
    ble t0, t5, 5f
    li t0, 20
5:  sw t0, 0(t6)
    j 2f
1:  li t2, 8
    neg t4, t4
    la t6, invaders_graphics_y
    lw t0, 0(t6)
    addi t0, t0, 8
    li t5, 96
    ble t0, t5, 6f
    li t0, 20
6:  sw t0, 0(t6)
2:  sw t2, 0(t1)
    sw t4, 0(t3)

    /* Player cannon auto patrol along bottom. */
    la t1, invaders_graphics_cannon_x
    lw t2, 0(t1)
    la t3, invaders_graphics_cannon_dx
    lw t4, 0(t3)
    add t2, t2, t4
    bltz t2, 3f
    li t5, 292
    ble t2, t5, 4f
    li t2, 292
    neg t4, t4
    j 4f
3:  li t2, 0
    neg t4, t4
4:  sw t2, 0(t1)
    sw t4, 0(t3)

    /* Shot climbs upward and respawns from cannon when it leaves screen. */
    la t1, invaders_graphics_shot_y
    lw t2, 0(t1)
    addi t2, t2, -4
    bgez t2, 7f
    li t2, 170
    la t3, invaders_graphics_shot_x
    la t4, invaders_graphics_cannon_x
    lw t5, 0(t4)
    addi t5, t5, 14
    sw t5, 0(t3)
7:  sw t2, 0(t1)

    la t1, invaders_graphics_score    /* Load score variable address. */
    lw t2, 0(t1)                  /* Load current score/counter. */
    addi t2, t2, 1                /* Increment counter once per frame. */
    sw t2, 0(t1)                  /* Store updated counter. */
    lw ra, 12(sp)                 /* Restore return address. */
    addi sp, sp, 16               /* Release stack frame. */
    ret                           /* Return to C scheduler. */

invaders_graphics_draw:
    addi sp, sp, -16              /* Save state before C drawing calls. */
    sw ra, 12(sp)                 /* Save return address. */
    li a0, 0                      /* RGB565 black background. */
    call prg32_gfx_clear          /* Clear 320x200 retro graphics viewport. */

    /* Player cannon near the bottom, follows x to keep visible motion. */
    la t0, invaders_graphics_cannon_x
    lw a0, 0(t0)
    li a1, 186
    li a2, 28
    li a3, 8
    li a4, 65535                  /* White cannon. */
    call prg32_gfx_rect

    /* Moving invader formation (three aliens) driven by x/y. */
    la t0, invaders_graphics_x
    lw a0, 0(t0)
    la t0, invaders_graphics_y
    lw a1, 0(t0)
    li a2, 14
    li a3, 10
    li a4, 2016                   /* Green invader #1. */
    call prg32_gfx_rect

    la t0, invaders_graphics_x
    lw a0, 0(t0)
    addi a0, a0, 26
    la t0, invaders_graphics_y
    lw a1, 0(t0)
    addi a1, a1, 4
    li a2, 14
    li a3, 10
    li a4, 2016                   /* Green invader #2. */
    call prg32_gfx_rect

    la t0, invaders_graphics_x
    lw a0, 0(t0)
    addi a0, a0, 52
    la t0, invaders_graphics_y
    lw a1, 0(t0)
    li a2, 14
    li a3, 10
    li a4, 2016                   /* Green invader #3. */
    call prg32_gfx_rect

    /* Player shot (white laser). */
    la t0, invaders_graphics_shot_x
    lw a0, 0(t0)
    la t0, invaders_graphics_shot_y
    lw a1, 0(t0)
    li a2, 2
    li a3, 8
    li a4, 65535
    call prg32_gfx_rect

    lw ra, 12(sp)                 /* Restore return address after drawing. */
    addi sp, sp, 16               /* Free stack frame. */
    ret                           /* Return to main loop. */
