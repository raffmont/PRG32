/*
 * ASTEROIDS GRAPHICS for PRG32.
 * This file is intentionally verbose: every instruction is commented so
 * first-year students can connect RISC-V assembly to visible hardware effects.
 */
.option norelax

.section .data
asteroids_graphics_x:      .word 156       /* Ship x coordinate in pixels. */
asteroids_graphics_y:      .word 182       /* Ship y coordinate in pixels. */
asteroids_graphics_ship_dx:.word 2         /* Ship patrol speed. */
asteroids_graphics_alien_x:.word 260       /* Alien ship x coordinate in pixels. */
asteroids_graphics_alien_y:.word 32        /* Alien ship y coordinate in pixels. */
asteroids_graphics_alien_dx:.word -2       /* Alien horizontal speed. */
asteroids_graphics_shot_x: .word 0         /* Laser shot x coordinate. */
asteroids_graphics_shot_y: .word 0         /* Laser shot y coordinate. */
asteroids_graphics_shot_on:.word 0         /* 1 while shot is active, 0 otherwise. */
asteroids_graphics_cooldown:.word 0        /* Frames before auto-firing again. */
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

    /* Ship patrol: x += ship_dx, bounce between screen edges. */
    la t1, asteroids_graphics_x         /* Load address of ship x. */
    lw t2, 0(t1)                        /* Load ship x. */
    la t3, asteroids_graphics_ship_dx
    lw t4, 0(t3)
    add t2, t2, t4
    bltz t2, .Lasteroids_ship_left
    li t5, 312
    ble t2, t5, .Lasteroids_ship_store
    li t2, 312
    neg t4, t4
    j .Lasteroids_ship_store
.Lasteroids_ship_left:
    li t2, 0
    neg t4, t4
.Lasteroids_ship_store:
    sw t2, 0(t1)
    sw t4, 0(t3)

    /* Alien patrol: x += alien_dx, wrap to right and swap lane on exit. */
    la t1, asteroids_graphics_alien_x
    lw t2, 0(t1)
    la t3, asteroids_graphics_alien_dx
    lw t4, 0(t3)
    add t2, t2, t4
    bgez t2, .Lasteroids_alien_store
    li t2, 319
    la t6, asteroids_graphics_alien_y
    lw t0, 0(t6)
    li t5, 32
    beq t0, t5, .Lasteroids_alien_lane2
    li t0, 32
    j .Lasteroids_alien_lane_store
.Lasteroids_alien_lane2:
    li t0, 72
.Lasteroids_alien_lane_store:
    sw t0, 0(t6)
.Lasteroids_alien_store:
    sw t2, 0(t1)

    /* Auto-fire: start a new shot when cooldown reaches zero. */
    la t1, asteroids_graphics_cooldown
    lw t2, 0(t1)
    beqz t2, .Lasteroids_spawn_shot
    addi t2, t2, -1
    sw t2, 0(t1)
    j .Lasteroids_update_shot
.Lasteroids_spawn_shot:
    la t3, asteroids_graphics_shot_on
    li t4, 1
    sw t4, 0(t3)
    la t3, asteroids_graphics_x
    lw t4, 0(t3)
    addi t4, t4, 4
    la t3, asteroids_graphics_shot_x
    sw t4, 0(t3)
    la t3, asteroids_graphics_y
    lw t4, 0(t3)
    addi t4, t4, -6
    la t3, asteroids_graphics_shot_y
    sw t4, 0(t3)
    la t3, asteroids_graphics_cooldown
    li t4, 22
    sw t4, 0(t3)

.Lasteroids_update_shot:
    la t1, asteroids_graphics_shot_on
    lw t2, 0(t1)
    beqz t2, .Lasteroids_score_tick
    la t3, asteroids_graphics_shot_y
    lw t4, 0(t3)
    addi t4, t4, -4
    bltz t4, .Lasteroids_shot_off
    sw t4, 0(t3)
    j .Lasteroids_hit_check
.Lasteroids_shot_off:
    sw zero, 0(t1)
    j .Lasteroids_score_tick

.Lasteroids_hit_check:
    la t3, asteroids_graphics_shot_x
    lw t4, 0(t3)                        /* shot_x */
    la t6, asteroids_graphics_alien_x
    lw t0, 0(t6)                        /* alien_x */
    blt t4, t0, .Lasteroids_score_tick
    addi t5, t0, 18
    bgt t4, t5, .Lasteroids_score_tick
    la t3, asteroids_graphics_shot_y
    lw t4, 0(t3)                        /* shot_y */
    la t6, asteroids_graphics_alien_y
    lw t0, 0(t6)                        /* alien_y */
    blt t4, t0, .Lasteroids_score_tick
    addi t5, t0, 12
    bgt t4, t5, .Lasteroids_score_tick

    /* Hit confirmed: remove shot, respawn alien, reward score, play beep. */
    la t3, asteroids_graphics_shot_on
    sw zero, 0(t3)
    la t3, asteroids_graphics_alien_x
    li t4, 319
    sw t4, 0(t3)
    la t3, asteroids_graphics_alien_y
    lw t4, 0(t3)
    li t5, 32
    beq t4, t5, .Lasteroids_hit_lane2
    li t4, 32
    j .Lasteroids_hit_lane_store
.Lasteroids_hit_lane2:
    li t4, 72
.Lasteroids_hit_lane_store:
    sw t4, 0(t3)
    la t3, asteroids_graphics_score
    lw t4, 0(t3)
    addi t4, t4, 100
    sw t4, 0(t3)
    li a0, 980
    li a1, 20
    call prg32_audio_beep

.Lasteroids_score_tick:
    la t1, asteroids_graphics_score     /* Load score address. */
    lw t2, 0(t1)                        /* Load score. */
    addi t2, t2, 1                      /* Count each frame. */
    sw t2, 0(t1)

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

    la t0, asteroids_graphics_alien_x   /* Load alien x address. */
    lw s2, 0(t0)                        /* Keep alien x in s2. */
    la t0, asteroids_graphics_alien_y   /* Load alien y address. */
    lw s3, 0(t0)                        /* Keep alien y in s3. */

    mv a0, s2                           /* Alien body x. */
    mv a1, s3                           /* Alien body y. */
    li a2, 20                           /* Alien width. */
    li a3, 12                           /* Alien height. */
    li a4, 2016                         /* RGB565 green. */
    call prg32_gfx_rect                 /* Draw alien ship body. */

    addi a0, s2, 4                      /* Alien cockpit x. */
    addi a1, s3, -3                     /* Alien cockpit y. */
    li a2, 10                           /* Cockpit width. */
    li a3, 3                            /* Cockpit height. */
    li a4, 65535                        /* White cockpit highlight. */
    call prg32_gfx_rect

    /* Draw the laser when shot_on == 1. */
    la t0, asteroids_graphics_shot_on
    lw t1, 0(t0)
    beqz t1, .Lasteroids_draw_done
    la t0, asteroids_graphics_shot_x
    lw a0, 0(t0)
    la t0, asteroids_graphics_shot_y
    lw a1, 0(t0)
    li a2, 2
    li a3, 8
    li a4, 65535
    call prg32_gfx_rect
.Lasteroids_draw_done:

    lw s3, 12(sp)                       /* Restore s3. */
    lw s2, 16(sp)                       /* Restore s2. */
    lw s1, 20(sp)                       /* Restore s1. */
    lw s0, 24(sp)                       /* Restore s0. */
    lw ra, 28(sp)                       /* Restore return address. */
    addi sp, sp, 32                     /* Release stack frame. */
    ret                                 /* Return to main loop. */
