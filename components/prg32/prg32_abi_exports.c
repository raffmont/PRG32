#include "prg32.h"
#include <stddef.h>
#include <stdint.h>

/*
 * Keep the cartridge ABI entry points alive in firmware ELF output.
 * The default app does not call all of these symbols directly, so
 * --gc-sections can otherwise strip them.
 */
typedef void (*prg32_any_fn_t)(void);

static const prg32_any_fn_t g_prg32_cart_abi_exports[] = {
    (prg32_any_fn_t)prg32_ticks_ms,
    (prg32_any_fn_t)prg32_input_read,
    (prg32_any_fn_t)prg32_audio_beep,
    (prg32_any_fn_t)prg32_console_hex32,
    (prg32_any_fn_t)prg32_gfx_pixel,
    (prg32_any_fn_t)prg32_gfx_rect,
    (prg32_any_fn_t)prg32_debug_overlay_draw,
    (prg32_any_fn_t)prg32_tile_clear,
    (prg32_any_fn_t)prg32_tile_define,
    (prg32_any_fn_t)prg32_tile_put,
    (prg32_any_fn_t)prg32_tile_present,
    (prg32_any_fn_t)prg32_sprite_hitbox,
    (prg32_any_fn_t)prg32_sprite_draw_8x8,
    (prg32_any_fn_t)prg32_sprite_draw_16x16,
    (prg32_any_fn_t)prg32_score_submit,
};

void prg32_abi_exports_keep(void) {
    volatile uintptr_t sink = 0;
    for (size_t i = 0; i < sizeof(g_prg32_cart_abi_exports) / sizeof(g_prg32_cart_abi_exports[0]); ++i) {
        sink ^= (uintptr_t)g_prg32_cart_abi_exports[i];
    }
    (void)sink;
}
