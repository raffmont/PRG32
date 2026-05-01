/*
 * ASTEROIDS GRAPHICS for PRG32.
 * This file is intentionally verbose: every instruction is commented so
 * first-year students can connect RISC-V assembly to visible hardware effects.
 */
.option norelax

.section .data
asteroids_graphics_x:      .word 156       /* Ship x coordinate in pixels. */
asteroids_graphics_y:      .word 96        /* Ship y coordinate in pixels. */
asteroids_graphics_rock_x: .word 260       /* Asteroid x coordinate in pixels. */
asteroids_graphics_rock_y: .word 40        /* Asteroid y coordinate in pixels. */
asteroids_graphics_score:  .word 0         /* Score/frame counter. */

.section .rodata
asteroids_graphics_title: .asciz "ASTEROIDS GRAPHICS\n"
asteroids_graphics_msg:   .asciz "ARROWS MOVE, A SCORES\n"

.section .text
.global asteroids_graphics_init
.global asteroids_graphics_update
.global asteroids_graphics_draw

asteroids_graphics_init:
    addi sp, sp, -16                    /* Build a small stack frame. */
    sw ra, 12(sp)                       /* Save return address for C calls. */
    call prg32_console_clear            /* Clear text/debug output. */
    la a0, asteroids_graphics_title     /* Load title pointer. */
    call prg32_console_write            /* Print title. */
    la a0, asteroids_graphics_msg       /* Load help pointer. */
    call prg32_console_write            /* Print help. */
    li a0, 660                          /* Startup beep frequency. */
    li a1, 40                           /* Startup beep duration. */
    call prg32_audio_beep               /* Play startup beep. */
    lw ra, 12(sp)                       /* Restore return address. */
    addi sp, sp, 16                     /* Release stack frame. */
    ret                                 /* Return to C game loop. */

asteroids_graphics_update:
    addi sp, sp, -16                    /* Save state before framework call. */
    sw ra, 12(sp)                       /* Preserve return address. */
    call prg32_input_read               /* Read input mask into a0. */
    mv t0, a0                           /* Copy input into t0. */

    la t1, asteroids_graphics_x         /* Load address of ship x. */
    lw t2, 0(t1)                        /* Load ship x. */
    andi t3, t0, 1                      /* Test PRG32_BTN_LEFT. */
    beqz t3, 1f                         /* Skip if left is not pressed. */
    addi t2, t2, -4                     /* Move ship left. */
1:  andi t3, t0, 2                      /* Test PRG32_BTN_RIGHT. */
    beqz t3, 2f                         /* Skip if right is not pressed. */
    addi t2, t2, 4                      /* Move ship right. */
2:  bltz t2, 3f                         /* Clamp negative x. */
    li t3, 312                          /* Right edge for 8-pixel ship. */
    ble t2, t3, 4f                      /* Keep x if in range. */
    mv t2, t3                           /* Clamp to right edge. */
    j 4f                                /* Continue. */
3:  li t2, 0                            /* Clamp to left edge. */
4:  sw t2, 0(t1)                        /* Store ship x. */

    la t1, asteroids_graphics_y         /* Load address of ship y. */
    lw t2, 0(t1)                        /* Load ship y. */
    andi t3, t0, 4                      /* Test PRG32_BTN_UP. */
    beqz t3, 5f                         /* Skip if up is not pressed. */
    addi t2, t2, -4                     /* Move ship up. */
5:  andi t3, t0, 8                      /* Test PRG32_BTN_DOWN. */
    beqz t3, 6f                         /* Skip if down is not pressed. */
    addi t2, t2, 4                      /* Move ship down. */
6:  bltz t2, 7f                         /* Clamp negative y. */
    li t3, 192                          /* Bottom edge for 8-pixel ship. */
    ble t2, t3, 8f                      /* Keep y if in range. */
    mv t2, t3                           /* Clamp to bottom edge. */
    j 8f                                /* Continue. */
7:  li t2, 0                            /* Clamp to top edge. */
8:  sw t2, 0(t1)                        /* Store ship y. */

    la t1, asteroids_graphics_rock_x    /* Load address of asteroid x. */
    lw t2, 0(t1)                        /* Load asteroid x. */
    addi t2, t2, -2                     /* Drift asteroid left each frame. */
    bgez t2, 9f                         /* If still visible, keep it. */
    li t2, 319                          /* Wrap asteroid to right edge. */
9:  sw t2, 0(t1)                        /* Store asteroid x. */

    la t1, asteroids_graphics_score     /* Load score address. */
    lw t2, 0(t1)                        /* Load score. */
    addi t2, t2, 1                      /* Count each frame. */
    andi t3, t0, 16                     /* Test PRG32_BTN_A. */
    beqz t3, .Lasteroids_graphics_store_score /* Skip bonus/beep if A is not pressed. */
    addi t2, t2, 25                     /* Add fire/action score bonus. */
    sw t2, 0(t1)                        /* Store score before blocking beep. */
    li a0, 880                          /* Fire beep frequency. */
    li a1, 15                           /* Fire beep duration. */
    call prg32_audio_beep               /* Play short action sound. */
    j .Lasteroids_graphics_update_done  /* Score already stored. */
.Lasteroids_graphics_store_score:
    sw t2, 0(t1)                        /* Store regular frame score. */

.Lasteroids_graphics_update_done:
    lw ra, 12(sp)                       /* Restore return address. */
    addi sp, sp, 16                     /* Release stack frame. */
    ret                                 /* Return to scheduler. */

asteroids_graphics_draw:
    addi sp, sp, -32                    /* Save ra and s registers used across C calls. */
    sw ra, 28(sp)                       /* Save return address. */
    sw s0, 24(sp)                       /* Save callee-saved register s0. */
    sw s1, 20(sp)                       /* Save callee-saved register s1. */
    sw s2, 16(sp)                       /* Save callee-saved register s2. */
    sw s3, 12(sp)                       /* Save callee-saved register s3. */

    li a0, 0                            /* RGB565 black background. */
    call prg32_gfx_clear                /* Clear the 320x200 game viewport. */

    la t0, asteroids_graphics_x         /* Load ship x address. */
    lw s0, 0(t0)                        /* Keep ship x in s0 across calls. */
    la t0, asteroids_graphics_y         /* Load ship y address. */
    lw s1, 0(t0)                        /* Keep ship y in s1 across calls. */

    mv a0, s0                           /* Ship nose x. */
    mv a1, s1                           /* Ship nose y. */
    li a2, 8                            /* Nose width. */
    li a3, 3                            /* Nose height. */
    li a4, 65535                        /* RGB565 white. */
    call prg32_gfx_rect                 /* Draw ship nose. */

    addi a0, s0, 2                      /* Ship body x. */
    addi a1, s1, 3                      /* Ship body y. */
    li a2, 4                            /* Body width. */
    li a3, 8                            /* Body height. */
    li a4, 65535                        /* RGB565 white. */
    call prg32_gfx_rect                 /* Draw ship body. */

    la t0, asteroids_graphics_rock_x    /* Load asteroid x address. */
    lw s2, 0(t0)                        /* Keep asteroid x in s2. */
    la t0, asteroids_graphics_rock_y    /* Load asteroid y address. */
    lw s3, 0(t0)                        /* Keep asteroid y in s3. */

    mv a0, s2                           /* Asteroid main x. */
    mv a1, s3                           /* Asteroid main y. */
    li a2, 18                           /* Asteroid main width. */
    li a3, 12                           /* Asteroid main height. */
    li a4, 33808                        /* RGB565 gray. */
    call prg32_gfx_rect                 /* Draw asteroid block. */

    addi a0, s2, 5                      /* Asteroid highlight x. */
    addi a1, s3, -4                     /* Asteroid highlight y. */
    li a2, 8                            /* Highlight width. */
    li a3, 4                            /* Highlight height. */
    li a4, 50712                        /* RGB565 lighter gray. */
    call prg32_gfx_rect                 /* Draw uneven asteroid top. */

    lw s3, 12(sp)                       /* Restore s3. */
    lw s2, 16(sp)                       /* Restore s2. */
    lw s1, 20(sp)                       /* Restore s1. */
    lw s0, 24(sp)                       /* Restore s0. */
    lw ra, 28(sp)                       /* Restore return address. */
    addi sp, sp, 32                     /* Release stack frame. */
    ret                                 /* Return to main loop. */
