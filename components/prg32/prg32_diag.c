#include "prg32.h"

static volatile uint32_t g_diag_input_state;
static volatile uint32_t g_diag_frame_count;

void prg32_diag_set_input_state(uint32_t input_state) {
    g_diag_input_state = input_state;
}

void prg32_diag_increment_frame(void) {
    g_diag_frame_count++;
}

uint32_t prg32_diag_input_state(void) {
    return g_diag_input_state;
}

uint32_t prg32_diag_frame_count(void) {
    return g_diag_frame_count;
}
