/*
 * PAC-MAN GRAPHICS for URG32.
 * This file is intentionally verbose: every instruction is commented so
 * first-year students can connect RISC-V assembly to visible hardware effects.
 */
.option norelax
.section .data
pacman_graphics_x:      .word 20          /* Object x coordinate in pixels or text columns. */
pacman_graphics_y:      .word 20          /* Object y coordinate in pixels or text rows. */
pacman_graphics_dx:     .word 2           /* Horizontal velocity. */
pacman_graphics_dy:     .word 1           /* Vertical velocity. */
pacman_graphics_score:  .word 0           /* Simple score/counter. */

.section .rodata
pacman_graphics_title: .asciz "PAC-MAN GRAPHICS\n"
pacman_graphics_msg:   .asciz "URG32 RISC-V ASM DEMO\n"

.section .text
.global pacman_graphics_init
.global pacman_graphics_update
.global pacman_graphics_draw

pacman_graphics_init:
    addi sp, sp, -16              /* Build a tiny stack frame before calling C. */
    sw ra, 12(sp)                 /* Save return address because call overwrites ra. */
    call urg32_console_clear      /* Clear UART/LCD text console. */
    la a0, pacman_graphics_title        /* Put address of title string into argument a0. */
    call urg32_console_write      /* Print title through framework console. */
    la a0, pacman_graphics_msg          /* Load second message pointer into a0. */
    call urg32_console_write      /* Print explanatory message. */
    li a0, 120                    /* Choose a short beep frequency/duration argument. */
    li a1, 30                     /* 30 milliseconds: audible but not annoying. */
    call urg32_audio_beep         /* Call C PWM audio helper. */
    lw ra, 12(sp)                 /* Restore return address. */
    addi sp, sp, 16               /* Destroy stack frame. */
    ret                           /* Return to the C game loop. */

pacman_graphics_update:
    addi sp, sp, -16              /* Save caller state before using C functions. */
    sw ra, 12(sp)                 /* Preserve return address. */
    call urg32_input_read         /* Read buttons; result appears in a0. */
    mv t0, a0                     /* Copy button bitmask into temporary register t0. */
    la t1, pacman_graphics_x            /* Load address of x variable. */
    lw t2, 0(t1)                  /* Load current x value. */
    andi t3, t0, 1                /* Test LEFT button bit. */
    beqz t3, 1f                   /* If not pressed, skip left movement. */
    addi t2, t2, -3               /* Move object left by 3 pixels/cells. */
1:  andi t3, t0, 2                /* Test RIGHT button bit. */
    beqz t3, 2f                   /* If not pressed, skip right movement. */
    addi t2, t2, 3                /* Move object right by 3 pixels/cells. */
2:  sw t2, 0(t1)                  /* Store new x coordinate. */
    la t1, pacman_graphics_score        /* Load score variable address. */
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
    call urg32_gfx_clear          /* Clear 320x200 retro graphics viewport. */
    la t0, pacman_graphics_x            /* Load address of x coordinate. */
    lw a0, 0(t0)                  /* a0 = x position. */
    li a1, 90                     /* a1 = y position. */
    li a2, 32                     /* a2 = rectangle width. */
    li a3, 12                     /* a3 = rectangle height. */
    li a4, 65535                  /* a4 = white color in RGB565. */
    call urg32_gfx_rect           /* Draw one moving arcade object. */
    lw ra, 12(sp)                 /* Restore return address after drawing. */
    addi sp, sp, 16               /* Free stack frame. */
    ret                           /* Return to main loop. */
