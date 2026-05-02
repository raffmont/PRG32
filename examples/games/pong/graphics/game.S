/*
 * PONG GRAPHICS for PRG32.
 * This file is intentionally verbose: every instruction is commented so
 * first-year students can connect RISC-V assembly to visible hardware effects.
 */
#include "prg32_config.h"
.option norelax
.section .data
pong_graphics_paddle_x: .word 120       /* Paddle x coordinate in pixels. */
pong_graphics_paddle_dx:.word 2         /* Paddle horizontal velocity. */
pong_graphics_ball_x:   .word 156       /* Ball x coordinate in pixels. */
pong_graphics_ball_y:   .word 96        /* Ball y coordinate in pixels. */
pong_graphics_ball_dx:  .word 2         /* Ball horizontal velocity. */
pong_graphics_ball_dy:  .word 1         /* Ball vertical velocity. */
pong_graphics_score:  .word 0           /* Simple score/counter. */

.section .rodata
pong_graphics_title: .asciz "PONG GRAPHICS\n"
pong_graphics_msg:   .asciz "PRG32 RISC-V ASM DEMO\n"

.section .text
.global pong_graphics_init
.global pong_graphics_update
.global pong_graphics_draw

pong_graphics_init:
    /* INIT: runs once when the cartridge is selected. */
    addi sp, sp, -16              /* Build a tiny stack frame before calling C. */
    sw ra, 12(sp)                 /* Save return address because call overwrites ra. */
    call prg32_console_clear      /* Clear UART/LCD text console. */
    la a0, pong_graphics_title    /* Put address of title string into argument a0. */
    call prg32_console_write      /* Print title through framework console. */
    la a0, pong_graphics_msg      /* Load second message pointer into a0. */
    call prg32_console_write      /* Print explanatory message. */
    li a0, 120                    /* Choose a short beep frequency/duration argument. */
    li a1, 30                     /* 30 milliseconds: audible but not annoying. */
    call prg32_audio_beep         /* Call C PWM audio helper. */
    lw ra, 12(sp)                 /* Restore return address. */
    addi sp, sp, 16               /* Destroy stack frame. */
    ret                           /* Return to the C game loop. */

pong_graphics_update:
    /*
     * UPDATE: run autonomous demo motion so QEMU always shows activity.
     * Paddle moves left/right and the ball bounces on all viewport edges.
     */
    addi sp, sp, -16              /* Keep a normal call-safe frame style. */
    sw ra, 12(sp)                 /* Preserve return address. */

    /* Paddle auto motion: x += dx, bounce at [0, 256] for 64-pixel paddle. */
    la t1, pong_graphics_paddle_x
    lw t2, 0(t1)                  /* t2 = paddle x */
    la t3, pong_graphics_paddle_dx
    lw t4, 0(t3)                  /* t4 = paddle dx */
    add t2, t2, t4
    bltz t2, 1f                   /* Left wall hit. */
    li t5, 256                    /* 320 - paddle width (64). */
    ble t2, t5, 2f
    mv t2, t5                     /* Right wall clamp. */
    neg t4, t4                    /* Reverse direction. */
    j 2f
1:  li t2, 0                      /* Left wall clamp. */
    neg t4, t4                    /* Reverse direction. */
2:  sw t2, 0(t1)
    sw t4, 0(t3)

    /* Ball autonomous motion: x/y += dx/dy with edge bounces. */
    la t1, pong_graphics_ball_x
    lw t2, 0(t1)                  /* t2 = ball x */
    la t3, pong_graphics_ball_dx
    lw t4, 0(t3)                  /* t4 = ball dx */
    add t2, t2, t4
    bltz t2, 3f
    li t5, 312                    /* 320 - ball width (8). */
    ble t2, t5, 4f
    mv t2, t5
    neg t4, t4
    j 4f
3:  li t2, 0
    neg t4, t4
4:  sw t2, 0(t1)
    sw t4, 0(t3)

    la t1, pong_graphics_ball_y
    lw t2, 0(t1)                  /* t2 = ball y */
    la t3, pong_graphics_ball_dy
    lw t4, 0(t3)                  /* t4 = ball dy */
    add t2, t2, t4
    bltz t2, 5f
    li t5, 192                    /* 200 - ball height (8). */
    ble t2, t5, 6f
    mv t2, t5
    neg t4, t4
    j 6f
5:  li t2, 0
    neg t4, t4
6:  sw t2, 0(t1)
    sw t4, 0(t3)

    la t1, pong_graphics_score    /* Load score variable address. */
    lw t2, 0(t1)                  /* Load current score/counter. */
    addi t2, t2, 1                /* Increment counter once per frame. */
    sw t2, 0(t1)                  /* Store updated counter. */
    lw ra, 12(sp)                 /* Restore return address. */
    addi sp, sp, 16               /* Release stack frame. */
    ret                           /* Return to C scheduler. */

pong_graphics_draw:
    /* DRAW: runs every frame and renders the current game state. */
    addi sp, sp, -16              /* Save state before C drawing calls. */
    sw ra, 12(sp)                 /* Save return address. */
    li a0, 0                      /* RGB565 black background. */
    call prg32_gfx_clear          /* Clear 320x200 retro graphics viewport. */

    la t0, pong_graphics_paddle_x /* Paddle x coordinate. */
    lw a0, 0(t0)
    li a1, 188                    /* Paddle stays near bottom for visibility. */
    li a2, 64                     /* Wide paddle is easy to see in QEMU. */
    li a3, 8
    li a4, 65535                  /* a4 = white color in RGB565. */
    call prg32_gfx_rect           /* Draw auto-moving paddle. */

    la t0, pong_graphics_ball_x   /* Ball x coordinate. */
    lw a0, 0(t0)
    la t0, pong_graphics_ball_y   /* Ball y coordinate. */
    lw a1, 0(t0)
    li a2, 8
    li a3, 8
    li a4, 65504                  /* Yellow ball. */
    call prg32_gfx_rect           /* Draw bouncing ball. */

#if PRG32_DEBUG
    /*
     * Optional debug HUD example:
     *   prg32_debug_overlay_draw(1, x, y, input_mask, frame_counter)
     */
    call prg32_input_read         /* Read current input for HUD. */
    mv t1, a0                     /* Keep input mask in t1. */
    la t0, pong_graphics_score    /* Reuse score as a tiny frame counter. */
    lw t2, 0(t0)
    li a0, 1                      /* Enable overlay. */
    la t0, pong_graphics_paddle_x
    lw a1, 0(t0)                  /* x */
    li a2, 188                    /* y */
    mv a3, t1                     /* input mask */
    mv a4, t2                     /* frame/counter */
    call prg32_debug_overlay_draw
#endif

    lw ra, 12(sp)                 /* Restore return address after drawing. */
    addi sp, sp, 16               /* Free stack frame. */
    ret                           /* Return to main loop. */
