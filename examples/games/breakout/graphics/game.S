/*
 * BREAKOUT GRAPHICS for PRG32.
 * This file is intentionally verbose: every instruction is commented so
 * first-year students can connect RISC-V assembly to visible hardware effects.
 */
.option norelax
.section .data
breakout_graphics_paddle_x: .word 120       /* Paddle x coordinate in pixels. */
breakout_graphics_paddle_dx:.word 2         /* Paddle horizontal velocity. */
breakout_graphics_ball_x:   .word 156       /* Ball x coordinate in pixels. */
breakout_graphics_ball_y:   .word 120       /* Ball y coordinate in pixels. */
breakout_graphics_ball_dx:  .word 2         /* Ball horizontal velocity. */
breakout_graphics_ball_dy:  .word -2        /* Ball vertical velocity. */
breakout_graphics_bricks:   .word 7         /* 3-bit mask for top brick row. */
breakout_graphics_score:  .word 0           /* Simple score/counter. */

.section .rodata
breakout_graphics_title: .asciz "BREAKOUT GRAPHICS\n"
breakout_graphics_msg:   .asciz "PRG32 RISC-V ASM DEMO\n"

.section .text
.global breakout_graphics_init
.global breakout_graphics_update
.global breakout_graphics_draw

breakout_graphics_init:
    addi sp, sp, -16              /* Build a tiny stack frame before calling C. */
    sw ra, 12(sp)                 /* Save return address because call overwrites ra. */
    call prg32_console_clear      /* Clear UART/LCD text console. */
    la a0, breakout_graphics_title    /* Put address of title string into argument a0. */
    call prg32_console_write      /* Print title through framework console. */
    la a0, breakout_graphics_msg      /* Load second message pointer into a0. */
    call prg32_console_write      /* Print explanatory message. */
    li a0, 120                    /* Choose a short beep frequency/duration argument. */
    li a1, 30                     /* 30 milliseconds: audible but not annoying. */
    call prg32_audio_beep         /* Call C PWM audio helper. */
    lw ra, 12(sp)                 /* Restore return address. */
    addi sp, sp, 16               /* Destroy stack frame. */
    ret                           /* Return to the C game loop. */

breakout_graphics_update:
    addi sp, sp, -16              /* Save caller state before update work. */
    sw ra, 12(sp)                 /* Preserve return address. */

    /* Paddle auto motion: x += dx, bounce inside [0, 264] (56-pixel paddle). */
    la t1, breakout_graphics_paddle_x
    lw t2, 0(t1)
    la t3, breakout_graphics_paddle_dx
    lw t4, 0(t3)
    add t2, t2, t4
    bltz t2, 1f
    li t5, 264
    ble t2, t5, 2f
    mv t2, t5
    neg t4, t4
    j 2f
1:  li t2, 0
    neg t4, t4
2:  sw t2, 0(t1)
    sw t4, 0(t3)

    /* Ball x update with wall bounce. */
    la t1, breakout_graphics_ball_x
    lw t2, 0(t1)
    la t3, breakout_graphics_ball_dx
    lw t4, 0(t3)
    add t2, t2, t4
    bltz t2, 3f
    li t5, 312
    ble t2, t5, 4f
    mv t2, t5
    neg t4, t4
    j 4f
3:  li t2, 0
    neg t4, t4
4:  sw t2, 0(t1)
    sw t4, 0(t3)

    /* Ball y update with ceiling bounce and life reset at bottom. */
    la t1, breakout_graphics_ball_y
    lw t2, 0(t1)
    la t3, breakout_graphics_ball_dy
    lw t4, 0(t3)
    add t2, t2, t4
    bltz t2, 5f
    li t5, 196
    ble t2, t5, 6f
    /* Ball fell below paddle: reset to center heading upward. */
    li t2, 120
    li t4, -2
    la t6, breakout_graphics_ball_x
    li t0, 156
    sw t0, 0(t6)
    j 6f
5:  li t2, 0
    neg t4, t4
6:  sw t2, 0(t1)
    sw t4, 0(t3)

    /*
     * Paddle collision:
     * if ball is near y=188 and x inside [paddle_x-8, paddle_x+56], bounce up.
     */
    li t5, 180
    blt t2, t5, 7f
    li t5, 191
    bgt t2, t5, 7f
    la t6, breakout_graphics_ball_x
    lw t0, 0(t6)                  /* t0 = ball_x */
    la t6, breakout_graphics_paddle_x
    lw t1, 0(t6)                  /* t1 = paddle_x */
    addi t5, t1, -8
    blt t0, t5, 7f
    addi t5, t1, 56
    bgt t0, t5, 7f
    la t6, breakout_graphics_ball_dy
    lw t4, 0(t6)
    bltz t4, 7f                   /* Already moving up, skip. */
    neg t4, t4
    sw t4, 0(t6)
7:
    /*
     * Brick collision on top row (three bricks):
     * x ranges: [20..99], [120..199], [220..299], y range [18..30].
     */
    la t6, breakout_graphics_ball_y
    lw t2, 0(t6)
    li t5, 18
    blt t2, t5, 8f
    li t5, 30
    bgt t2, t5, 8f
    la t6, breakout_graphics_ball_x
    lw t0, 0(t6)
    la t6, breakout_graphics_bricks
    lw t1, 0(t6)                  /* t1 = brick bitmask */

    andi t5, t1, 1
    beqz t5, 9f
    li t5, 20
    blt t0, t5, 9f
    li t5, 99
    bgt t0, t5, 9f
    li t5, -2
    and t1, t1, t5                /* Clear bit 0. */
    sw t1, 0(t6)
    j .Lbreakout_brick_hit
9:  andi t5, t1, 2
    beqz t5, .Lbreakout_brick2_check
    li t5, 120
    blt t0, t5, .Lbreakout_brick2_check
    li t5, 199
    bgt t0, t5, .Lbreakout_brick2_check
    li t5, -3
    and t1, t1, t5                /* Clear bit 1. */
    sw t1, 0(t6)
    j .Lbreakout_brick_hit
.Lbreakout_brick2_check:
    andi t5, t1, 4
    beqz t5, 8f
    li t5, 220
    blt t0, t5, 8f
    li t5, 299
    bgt t0, t5, 8f
    li t5, -5
    and t1, t1, t5                /* Clear bit 2. */
    sw t1, 0(t6)
.Lbreakout_brick_hit:
    /* Any brick hit: bounce ball and add score. */
    la t6, breakout_graphics_ball_dy
    lw t4, 0(t6)
    neg t4, t4
    sw t4, 0(t6)
    la t6, breakout_graphics_score
    lw t0, 0(t6)
    addi t0, t0, 10
    sw t0, 0(t6)
8:
    /* Respawn bricks after clearing all three. */
    la t6, breakout_graphics_bricks
    lw t1, 0(t6)
    bnez t1, .Lbreakout_bricks_keep
    li t1, 7
    sw t1, 0(t6)
.Lbreakout_bricks_keep:
    la t1, breakout_graphics_score    /* Load score variable address. */
    lw t2, 0(t1)                  /* Load current score/counter. */
    addi t2, t2, 1                /* Increment counter once per frame. */
    sw t2, 0(t1)                  /* Store updated counter. */
    lw ra, 12(sp)                 /* Restore return address. */
    addi sp, sp, 16               /* Release stack frame. */
    ret                           /* Return to C scheduler. */

breakout_graphics_draw:
    addi sp, sp, -16              /* Save state before C drawing calls. */
    sw ra, 12(sp)                 /* Save return address. */
    li a0, 0                      /* RGB565 black background. */
    call prg32_gfx_clear          /* Clear 320x200 retro graphics viewport. */

    /* Breakout paddle: auto-moving at the bottom. */
    la t0, breakout_graphics_paddle_x
    lw a0, 0(t0)
    li a1, 188
    li a2, 56
    li a3, 8
    li a4, 65535                  /* White paddle. */
    call prg32_gfx_rect

    /* Breakout ball: small bouncing block. */
    la t0, breakout_graphics_ball_x
    lw a0, 0(t0)
    la t0, breakout_graphics_ball_y
    lw a1, 0(t0)
    li a2, 8
    li a3, 8
    li a4, 65504                  /* Yellow ball. */
    call prg32_gfx_rect

    /* Draw active bricks from the 3-bit mask. */
    la t0, breakout_graphics_bricks
    lw t1, 0(t0)
    andi t2, t1, 1
    beqz t2, .Lbreakout_brick_draw_2
    li a0, 20
    li a1, 18
    li a2, 80
    li a3, 10
    li a4, 63488
    call prg32_gfx_rect
.Lbreakout_brick_draw_2:
    andi t2, t1, 2
    beqz t2, .Lbreakout_brick_draw_3
    li a0, 120
    li a1, 18
    li a2, 80
    li a3, 10
    li a4, 63488
    call prg32_gfx_rect
.Lbreakout_brick_draw_3:
    andi t2, t1, 4
    beqz t2, .Lbreakout_brick_draw_done
    li a0, 220
    li a1, 18
    li a2, 80
    li a3, 10
    li a4, 63488
    call prg32_gfx_rect
.Lbreakout_brick_draw_done:

    lw ra, 12(sp)                 /* Restore return address after drawing. */
    addi sp, sp, 16               /* Free stack frame. */
    ret                           /* Return to main loop. */
