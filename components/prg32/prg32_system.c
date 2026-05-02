#include "prg32.h"
#include "prg32_config.h"

void prg32_display_init(void);
void prg32_input_init(void);
void prg32_audio_init(void);

void prg32_init(void) {
    prg32_display_init();
    prg32_input_init();
    prg32_audio_init();
    prg32_cart_init();
    prg32_wifi_scores_init();
    prg32_scores_api_start();
    prg32_set_mode(PRG32_DEFAULT_MODE);
    prg32_console_clear();
}
