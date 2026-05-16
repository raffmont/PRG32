#include "prg32.h"
#include "prg32_config.h"

void prg32_display_init(void);
void prg32_input_init(void);
void prg32_audio_pwm_init(void);
void prg32_abi_exports_keep(void);

void prg32_init(void) {
    prg32_display_init();
    prg32_audio_pwm_init();
    prg32_splash_show_default();
    prg32_input_init();
    prg32_abi_exports_keep();
    prg32_cart_init();
    if (prg32_wifi_setup_requested()) {
        prg32_wifi_setup_run();
    } else {
        prg32_wifi_scores_init();
    }
    prg32_scores_api_start();
    prg32_set_mode(PRG32_DEFAULT_MODE);
    prg32_console_clear();
}
