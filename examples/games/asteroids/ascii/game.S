/*
 * ASTEROIDS ASCII for PRG32.
 * This file is intentionally verbose: every instruction is commented so
 * first-year students can connect RISC-V assembly to visible hardware effects.
 */
.option norelax

.section .data
asteroids_ascii_x:      .word 20       /* Ship x position in text-style coordinates. */
asteroids_ascii_y:      .word 12       /* Ship y position in text-style coordinates. */
asteroids_ascii_score:  .word 0        /* Score/frame counter shown as hexadecimal. */

.section .rodata
asteroids_ascii_title: .asciz "ASTEROIDS ASCII\n"
asteroids_ascii_msg:   .asciz "ARROWS MOVE, A SCORES\n"
asteroids_ascii_x_lbl: .asciz "X="
asteroids_ascii_y_lbl: .asciz " Y="
asteroids_ascii_s_lbl: .asciz " SCORE="

.section .text
.global asteroids_ascii_init
.global asteroids_ascii_update
.global asteroids_ascii_draw

asteroids_ascii_init:
    addi sp, sp, -16              /* Make a stack frame before calling C helpers. */
    sw ra, 12(sp)                 /* Save return address because call overwrites ra. */
    call prg32_console_clear      /* Clear UART/LCD text console. */
    la a0, asteroids_ascii_title  /* Put title string address into a0. */
    call prg32_console_write      /* Print title. */
    la a0, asteroids_ascii_msg    /* Put help string address into a0. */
    call prg32_console_write      /* Print help. */
    li a0, 660                    /* a0 = beep frequency in Hz. */
    li a1, 40                     /* a1 = duration in milliseconds. */
    call prg32_audio_beep         /* Short startup sound. */
    lw ra, 12(sp)                 /* Restore return address. */
    addi sp, sp, 16               /* Release stack frame. */
    ret                           /* Return to C game loop. */

asteroids_ascii_update:
    addi sp, sp, -16              /* Save state before calling the framework. */
    sw ra, 12(sp)                 /* Preserve return address. */
    call prg32_input_read         /* Read merged GPIO/controller input into a0. */
    mv t0, a0                     /* Copy input mask into t0. */

    la t1, asteroids_ascii_x      /* Load address of x variable. */
    lw t2, 0(t1)                  /* Load current x. */
    andi t3, t0, 1                /* Test PRG32_BTN_LEFT. */
    beqz t3, 1f                   /* Skip if left is not pressed. */
    addi t2, t2, -1               /* Move left one cell. */
1:  andi t3, t0, 2                /* Test PRG32_BTN_RIGHT. */
    beqz t3, 2f                   /* Skip if right is not pressed. */
    addi t2, t2, 1                /* Move right one cell. */
2:  bltz t2, 3f                   /* Clamp negative x back to zero. */
    li t3, 39                     /* Maximum text column. */
    ble t2, t3, 4f                /* Keep x if within bounds. */
    mv t2, t3                     /* Clamp x to right edge. */
    j 4f                          /* Skip low clamp. */
3:  li t2, 0                      /* Clamp x to left edge. */
4:  sw t2, 0(t1)                  /* Store updated x. */

    la t1, asteroids_ascii_y      /* Load address of y variable. */
    lw t2, 0(t1)                  /* Load current y. */
    andi t3, t0, 4                /* Test PRG32_BTN_UP. */
    beqz t3, 5f                   /* Skip if up is not pressed. */
    addi t2, t2, -1               /* Move up one row. */
5:  andi t3, t0, 8                /* Test PRG32_BTN_DOWN. */
    beqz t3, 6f                   /* Skip if down is not pressed. */
    addi t2, t2, 1                /* Move down one row. */
6:  bltz t2, 7f                   /* Clamp negative y back to zero. */
    li t3, 24                     /* Maximum text row. */
    ble t2, t3, 8f                /* Keep y if within bounds. */
    mv t2, t3                     /* Clamp y to bottom edge. */
    j 8f                          /* Skip low clamp. */
7:  li t2, 0                      /* Clamp y to top edge. */
8:  sw t2, 0(t1)                  /* Store updated y. */

    la t1, asteroids_ascii_score  /* Load address of score counter. */
    lw t2, 0(t1)                  /* Load current score. */
    addi t2, t2, 1                /* Count every frame. */
    andi t3, t0, 16               /* Test PRG32_BTN_A. */
    beqz t3, 9f                   /* Skip bonus if A is not pressed. */
    addi t2, t2, 15               /* Add score bonus for fire/action. */
    sw t2, 0(t1)                  /* Store score before blocking beep. */
    li a0, 880                    /* Fire beep frequency. */
    li a1, 15                     /* Fire beep duration. */
    call prg32_audio_beep         /* Play short action sound. */
    j .Lasteroids_ascii_update_done /* Score is already stored. */
9:  sw t2, 0(t1)                  /* Store regular frame score. */

.Lasteroids_ascii_update_done:
    lw ra, 12(sp)                 /* Restore return address. */
    addi sp, sp, 16               /* Release stack frame. */
    ret                           /* Return to scheduler. */

asteroids_ascii_draw:
    addi sp, sp, -16              /* Save state before drawing. */
    sw ra, 12(sp)                 /* Preserve return address. */
    call prg32_console_clear      /* Redraw the full text screen. */
    la a0, asteroids_ascii_title  /* Print title. */
    call prg32_console_write      /* Framework console write. */

    la a0, asteroids_ascii_x_lbl  /* Print X label. */
    call prg32_console_write      /* Framework console write. */
    la t0, asteroids_ascii_x      /* Load x variable address. */
    lw a0, 0(t0)                  /* Move x value into a0. */
    call prg32_console_hex32      /* Print x as hexadecimal. */

    la a0, asteroids_ascii_y_lbl  /* Print Y label. */
    call prg32_console_write      /* Framework console write. */
    la t0, asteroids_ascii_y      /* Load y variable address. */
    lw a0, 0(t0)                  /* Move y value into a0. */
    call prg32_console_hex32      /* Print y as hexadecimal. */

    la a0, asteroids_ascii_s_lbl  /* Print score label. */
    call prg32_console_write      /* Framework console write. */
    la t0, asteroids_ascii_score  /* Load score address. */
    lw a0, 0(t0)                  /* Move score into a0. */
    call prg32_console_hex32      /* Print score as hexadecimal. */
    li a0, 10                     /* ASCII newline. */
    call prg32_console_putc       /* Print newline. */

    lw ra, 12(sp)                 /* Restore return address. */
    addi sp, sp, 16               /* Release stack frame. */
    ret                           /* Return to main loop. */
