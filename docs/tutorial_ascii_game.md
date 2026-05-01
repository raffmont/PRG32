# Tutorial: writing an ASCII game in RISC-V assembly

1. Implement three symbols: `yourgame_init`, `yourgame_update`, and `yourgame_draw`.
2. Use `prg32_console_write` to print text.
3. Use `prg32_input_read` to read buttons.
4. Keep variables in `.data` and strings in `.rodata`.
5. Save `ra` before calling C functions.

```asm
.global demo_init
demo_init:
    addi sp, sp, -16
    sw ra, 12(sp)
    la a0, msg
    call prg32_console_write
    lw ra, 12(sp)
    addi sp, sp, 16
    ret

.section .rodata
msg: .asciz "Hello PRG32\n"
```
