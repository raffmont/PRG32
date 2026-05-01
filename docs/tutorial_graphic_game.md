# Tutorial: writing a graphic game in RISC-V assembly

The graphics mode exposes a 320x200 retro viewport.

Useful calls:

- `urg32_gfx_clear(color)`
- `urg32_gfx_pixel(x, y, color)`
- `urg32_gfx_rect(x, y, w, h, color)`
- `urg32_gfx_present()`
- `urg32_sprite_hitbox(ax, ay, aw, ah, bx, by, bw, bh)`

The framework internally tracks dirty rectangles, so students can draw only changed objects when they are ready for optimization exercises.
