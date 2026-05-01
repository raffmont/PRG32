#include "urg32.h"
#include "urg32_config.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void urg32_audio_init(void) {
    ledc_timer_config_t timer = {.speed_mode=LEDC_LOW_SPEED_MODE,.timer_num=LEDC_TIMER_0,.duty_resolution=LEDC_TIMER_10_BIT,.freq_hz=1000,.clk_cfg=LEDC_AUTO_CLK};
    ledc_channel_config_t ch = {.gpio_num=URG32_PIN_BUZZER,.speed_mode=LEDC_LOW_SPEED_MODE,.channel=LEDC_CHANNEL_0,.timer_sel=LEDC_TIMER_0,.duty=0,.hpoint=0};
    ledc_timer_config(&timer);
    ledc_channel_config(&ch);
}

void urg32_audio_beep(uint32_t hz, uint32_t ms) {
    if (!hz || !ms) return;
    ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, hz);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 512);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    vTaskDelay(pdMS_TO_TICKS(ms));
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}
