# Tutorial: writing a graphic game in RISC-V assembly

The graphics mode exposes a 320x200 retro viewport.

Useful calls:

- `prg32_gfx_clear(color)`
- `prg32_gfx_pixel(x, y, color)`
- `prg32_gfx_rect(x, y, w, h, color)`
- `prg32_gfx_present()`
- `prg32_sprite_hitbox(ax, ay, aw, ah, bx, by, bw, bh)`

The framework internally tracks dirty rectangles, so students can draw only changed objects when they are ready for optimization exercises.

The same calls work on both display backends:

- physical ILI9341 SPI TFT
- QEMU virtual RGB screen

For a desktop graphics test, run:

```bash
idf.py -B build-qemu -D SDKCONFIG_DEFAULTS=sdkconfig.defaults.qemu qemu --graphics monitor
```
