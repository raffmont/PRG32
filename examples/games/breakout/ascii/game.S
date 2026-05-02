/*
 * BREAKOUT ASCII for PRG32.
 * This file is intentionally verbose: every instruction is commented so
 * first-year students can connect RISC-V assembly to visible hardware effects.
 */
.option norelax
.section .data
breakout_ascii_x:      .word 20          /* Object x coordinate in text columns. */
breakout_ascii_y:      .word 12          /* Object y coordinate in text rows. */
breakout_ascii_dx:     .word 2           /* Horizontal velocity (teaching variable). */
breakout_ascii_dy:     .word 1           /* Vertical velocity (teaching variable). */
breakout_ascii_score:  .word 0           /* Simple score/counter. */

.section .rodata
breakout_ascii_title: .asciz "BREAKOUT ASCII\n"
breakout_ascii_msg:   .asciz "PRG32 RISC-V ASM DEMO\n"

.section .text
.global breakout_ascii_init
.global breakout_ascii_update
.global breakout_ascii_draw

breakout_ascii_init:
    addi sp, sp, -16              /* Build a tiny stack frame before calling C. */
    sw ra, 12(sp)                 /* Save return address because call overwrites ra. */
    call prg32_console_clear      /* Clear UART/LCD text console. */
    la a0, breakout_ascii_title       /* Put address of title string into argument a0. */
    call prg32_console_write      /* Print title through framework console. */
    la a0, breakout_ascii_msg         /* Load second message pointer into a0. */
    call prg32_console_write      /* Print explanatory message. */
    li a0, 120                    /* Choose a short beep frequency/duration argument. */
    li a1, 30                     /* 30 milliseconds: audible but not annoying. */
    call prg32_audio_beep         /* Call C PWM audio helper. */
    lw ra, 12(sp)                 /* Restore return address. */
    addi sp, sp, 16               /* Destroy stack frame. */
    ret                           /* Return to the C game loop. */

breakout_ascii_update:
    addi sp, sp, -16              /* Save caller state before using C functions. */
    sw ra, 12(sp)                 /* Preserve return address. */
    call prg32_input_read         /* Read buttons; result appears in a0. */
    mv t0, a0                     /* Copy button bitmask into temporary register t0. */

    la t1, breakout_ascii_x           /* Load address of x variable. */
    lw t2, 0(t1)                  /* Load current x value. */
    andi t3, t0, 1                /* Test LEFT button bit. */
    beqz t3, 1f                   /* If not pressed, skip left movement. */
    addi t2, t2, -1               /* Move object left by one text column. */
1:  andi t3, t0, 2                /* Test RIGHT button bit. */
    beqz t3, 2f                   /* If not pressed, skip right movement. */
    addi t2, t2, 1                /* Move object right by one text column. */
2:  bltz t2, 3f                   /* Clamp negative x back to zero. */
    li t3, 39                     /* Maximum text column. */
    ble t2, t3, 4f                /* Keep x when in range. */
    mv t2, t3                     /* Clamp to right edge. */
    j 4f
3:  li t2, 0                      /* Clamp to left edge. */
4:  sw t2, 0(t1)                  /* Store bounded x. */

    la t1, breakout_ascii_y           /* Load address of y variable. */
    lw t2, 0(t1)                  /* Load current y value. */
    andi t3, t0, 4                /* Test UP button bit. */
    beqz t3, 5f                   /* If not pressed, skip up movement. */
    addi t2, t2, -1               /* Move object up by one row. */
5:  andi t3, t0, 8                /* Test DOWN button bit. */
    beqz t3, 6f                   /* If not pressed, skip down movement. */
    addi t2, t2, 1                /* Move object down by one row. */
6:  bltz t2, 7f                   /* Clamp negative y back to zero. */
    li t3, 24                     /* Maximum text row. */
    ble t2, t3, 8f                /* Keep y when in range. */
    mv t2, t3                     /* Clamp to bottom edge. */
    j 8f
7:  li t2, 0                      /* Clamp to top edge. */
8:  sw t2, 0(t1)                  /* Store bounded y. */

    la t1, breakout_ascii_score       /* Load score variable address. */
    lw t2, 0(t1)                  /* Load current score/counter. */
    addi t2, t2, 1                /* Increment counter once per frame. */
    sw t2, 0(t1)                  /* Store updated counter. */
    lw ra, 12(sp)                 /* Restore return address. */
    addi sp, sp, 16               /* Release stack frame. */
    ret                           /* Return to C scheduler. */

breakout_ascii_draw:
    addi sp, sp, -16              /* Save state before C drawing calls. */
    sw ra, 12(sp)                 /* Save return address. */
    call prg32_console_clear      /* Text games redraw the logical ASCII screen. */
    la a0, breakout_ascii_title       /* Put title at top of console. */
    call prg32_console_write      /* Write title to current console backend. */
    la t0, breakout_ascii_score       /* Load score address. */
    lw a0, 0(t0)                  /* Load score value for debugging. */
    call prg32_console_hex32      /* Show score/counter as hexadecimal. */
    li a0, 10                     /* ASCII newline. */
    call prg32_console_putc       /* Print newline. */
    lw ra, 12(sp)                 /* Restore return address after drawing. */
    addi sp, sp, 16               /* Free stack frame. */
    ret                           /* Return to main loop. */
