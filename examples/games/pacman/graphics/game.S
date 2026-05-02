/*
 * PAC-MAN GRAPHICS for PRG32.
 * This file is intentionally verbose: every instruction is commented so
 * first-year students can connect RISC-V assembly to visible hardware effects.
 */
.option norelax
.section .data
pacman_graphics_x:      .word 20          /* Pac-Man x coordinate. */
pacman_graphics_y:      .word 110         /* Pac-Man y coordinate. */
pacman_graphics_dir:    .word 2           /* Horizontal direction (+2 / -2). */
pacman_graphics_ghost_x:.word 260         /* Ghost x coordinate. */
pacman_graphics_ghost_y:.word 110         /* Ghost y coordinate. */
pacman_graphics_pellet_x:.word 300        /* One moving pellet marker. */
pacman_graphics_score:  .word 0           /* Simple score/counter. */

.section .rodata
pacman_graphics_title: .asciz "PAC-MAN GRAPHICS\n"
pacman_graphics_msg:   .asciz "PRG32 RISC-V ASM DEMO\n"

.section .text
.global pacman_graphics_init
.global pacman_graphics_update
.global pacman_graphics_draw

pacman_graphics_init:
    addi sp, sp, -16              /* Build a tiny stack frame before calling C. */
    sw ra, 12(sp)                 /* Save return address because call overwrites ra. */
    call prg32_console_clear      /* Clear UART/LCD text console. */
    la a0, pacman_graphics_title    /* Put address of title string into argument a0. */
    call prg32_console_write      /* Print title through framework console. */
    la a0, pacman_graphics_msg      /* Load second message pointer into a0. */
    call prg32_console_write      /* Print explanatory message. */
    li a0, 120                    /* Choose a short beep frequency/duration argument. */
    li a1, 30                     /* 30 milliseconds: audible but not annoying. */
    call prg32_audio_beep         /* Call C PWM audio helper. */
    lw ra, 12(sp)                 /* Restore return address. */
    addi sp, sp, 16               /* Destroy stack frame. */
    ret                           /* Return to the C game loop. */

pacman_graphics_update:
    addi sp, sp, -16              /* Save caller state before update work. */
    sw ra, 12(sp)                 /* Preserve return address. */

    /* Pac-Man follows corridor: x += dir, bounce at side tunnel walls. */
    la t1, pacman_graphics_x
    lw t2, 0(t1)
    la t3, pacman_graphics_dir
    lw t4, 0(t3)
    add t2, t2, t4
    li t5, 10
    blt t2, t5, 1f
    li t5, 292
    ble t2, t5, 2f
    li t2, 292
    neg t4, t4
    j 2f
1:  li t2, 10
    neg t4, t4
2:  sw t2, 0(t1)
    sw t4, 0(t3)

    /* Keep Pac-Man on center corridor for readability. */
    la t1, pacman_graphics_y
    li t2, 110
    sw t2, 0(t1)

    /* Ghost chases from right to left with tunnel wrap. */
    la t1, pacman_graphics_ghost_x
    lw t2, 0(t1)
    addi t2, t2, -1
    bgez t2, 3f
    li t2, 312
3:  sw t2, 0(t1)

    la t1, pacman_graphics_ghost_y
    li t2, 110
    sw t2, 0(t1)

    /* Pellet train marker moves left and wraps. */
    la t1, pacman_graphics_pellet_x
    lw t2, 0(t1)
    addi t2, t2, -3
    bgez t2, 4f
    li t2, 320
4:  sw t2, 0(t1)

    la t1, pacman_graphics_score    /* Load score variable address. */
    lw t2, 0(t1)                  /* Load current score/counter. */
    addi t2, t2, 1                /* Increment counter once per frame. */
    sw t2, 0(t1)                  /* Store updated counter. */
    lw ra, 12(sp)                 /* Restore return address. */
    addi sp, sp, 16               /* Release stack frame. */
    ret                           /* Return to C scheduler. */

pacman_graphics_draw:
    addi sp, sp, -16              /* Save state before C drawing calls. */
    sw ra, 12(sp)                 /* Save return address. */
    li a0, 0                      /* RGB565 black background. */
    call prg32_gfx_clear          /* Clear 320x200 retro graphics viewport. */

    /* Draw simple maze lanes (blue bars). */
    li a0, 0
    li a1, 88
    li a2, 320
    li a3, 6
    li a4, 31                     /* Blue wall. */
    call prg32_gfx_rect

    li a0, 0
    li a1, 130
    li a2, 320
    li a3, 6
    li a4, 31
    call prg32_gfx_rect

    /* Pac-Man body: bright yellow moving block. */
    la t0, pacman_graphics_x
    lw a0, 0(t0)
    la t0, pacman_graphics_y
    lw a1, 0(t0)
    li a2, 18
    li a3, 18
    li a4, 65504                  /* Yellow hero. */
    call prg32_gfx_rect

    /* Pac-Man mouth notch: carve a black gap on the right side. */
    la t0, pacman_graphics_x
    lw a0, 0(t0)
    addi a0, a0, 12
    la t0, pacman_graphics_y
    lw a1, 0(t0)
    addi a1, a1, 6
    li a2, 6
    li a3, 6
    li a4, 0                      /* Black notch. */
    call prg32_gfx_rect

    /* Chasing ghost: independent position and color. */
    la t0, pacman_graphics_ghost_x
    lw a0, 0(t0)
    la t0, pacman_graphics_ghost_y
    lw a1, 0(t0)
    li a2, 14
    li a3, 14
    li a4, 2047                   /* Cyan ghost. */
    call prg32_gfx_rect

    /* Pellet dots across the lane using one scrolling anchor. */
    la t0, pacman_graphics_pellet_x
    lw t1, 0(t0)
    mv a0, t1
    li a1, 116
    li a2, 3
    li a3, 3
    li a4, 65535
    call prg32_gfx_rect

    addi a0, t1, -40
    li a1, 116
    li a2, 3
    li a3, 3
    li a4, 65535
    call prg32_gfx_rect

    addi a0, t1, -80
    li a1, 116
    li a2, 3
    li a3, 3
    li a4, 65535
    call prg32_gfx_rect

    lw ra, 12(sp)                 /* Restore return address after drawing. */
    addi sp, sp, 16               /* Free stack frame. */
    ret                           /* Return to main loop. */
