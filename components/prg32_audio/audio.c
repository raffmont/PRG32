#include "audio_internal.h"
#include <string.h>
#include "esp_timer.h"

prg32_audio_state_t g_prg32_audio;

static void audio_task(void *arg) {
    (void)arg;
    int16_t buffer[PRG32_AUDIO_MIX_FRAMES * 2];
    int64_t last_us = esp_timer_get_time();

    while (!g_prg32_audio.task_stop) {
        int64_t now_us = esp_timer_get_time();
        uint32_t elapsed_ms = (uint32_t)((now_us - last_us) / 1000);
        if (elapsed_ms > 0) {
            last_us = now_us;
            prg32_audio_tracker_step(elapsed_ms);
        }

        if (g_prg32_audio.config.mode == PRG32_AUDIO_MODE_STEREO) {
            prg32_audio_mix_stereo(buffer, PRG32_AUDIO_MIX_FRAMES);
        } else {
            prg32_audio_mix_mono(buffer, PRG32_AUDIO_MIX_FRAMES);
        }

        if (g_prg32_audio.i2s_ready) {
            prg32_audio_i2s_write(buffer,
                                  PRG32_AUDIO_MIX_FRAMES,
                                  g_prg32_audio.config.mode);
        } else {
            vTaskDelay(pdMS_TO_TICKS(4));
        }
    }

    g_prg32_audio.task = NULL;
    vTaskDelete(NULL);
}

static prg32_audio_config_t default_config(void) {
    prg32_audio_config_t config = {
        .sample_rate = CONFIG_PRG32_AUDIO_SAMPLE_RATE,
        .mode = PRG32_AUDIO_DEFAULT_MODE,
        .max_voices = CONFIG_PRG32_AUDIO_MAX_VOICES,
        .gpio_bclk = CONFIG_PRG32_AUDIO_I2S_BCLK_GPIO,
        .gpio_lrclk = CONFIG_PRG32_AUDIO_I2S_LRCLK_GPIO,
        .gpio_data = CONFIG_PRG32_AUDIO_I2S_DATA_GPIO,
        .gpio_sd = CONFIG_PRG32_AUDIO_I2S_SD_GPIO,
    };
    return config;
}

void prg32_audio_lock(void) {
    if (g_prg32_audio.lock) {
        xSemaphoreTake(g_prg32_audio.lock, portMAX_DELAY);
    }
}

void prg32_audio_unlock(void) {
    if (g_prg32_audio.lock) {
        xSemaphoreGive(g_prg32_audio.lock);
    }
}

bool prg32_audio_init(const prg32_audio_config_t *config) {
#if !CONFIG_PRG32_AUDIO_ENABLED
    (void)config;
    return false;
#else
    prg32_audio_config_t chosen = config ? *config : default_config();
    if (chosen.sample_rate == 0) {
        chosen.sample_rate = CONFIG_PRG32_AUDIO_SAMPLE_RATE;
    }
    if (chosen.mode != PRG32_AUDIO_MODE_STEREO) {
        chosen.mode = PRG32_AUDIO_MODE_MONO;
    }
    if (chosen.max_voices == 0 ||
        chosen.max_voices > CONFIG_PRG32_AUDIO_MAX_VOICES) {
        chosen.max_voices = CONFIG_PRG32_AUDIO_MAX_VOICES;
    }

    if (!g_prg32_audio.lock) {
        g_prg32_audio.lock = xSemaphoreCreateMutex();
    }

    prg32_audio_shutdown();
    memset(&g_prg32_audio, 0, sizeof(g_prg32_audio));
    g_prg32_audio.lock = xSemaphoreCreateMutex();
    g_prg32_audio.config = chosen;
    g_prg32_audio.master_volume = 255;
    g_prg32_audio.max_voices = chosen.max_voices;
    g_prg32_audio.tracker.tempo_bpm = 120;
    for (uint8_t i = 0; i < CONFIG_PRG32_AUDIO_MAX_VOICES; ++i) {
        g_prg32_audio.channel_volume[i] = 255;
        g_prg32_audio.channel_pan[i] = PRG32_AUDIO_PAN_CENTER;
    }

    g_prg32_audio.i2s_ready = prg32_audio_i2s_start(&chosen) == 0;
    g_prg32_audio.initialized = true;
    xTaskCreate(audio_task,
                "prg32_audio",
                4096,
                NULL,
                tskIDLE_PRIORITY + 2,
                &g_prg32_audio.task);
    return g_prg32_audio.i2s_ready;
#endif
}

void prg32_audio_shutdown(void) {
    if (g_prg32_audio.task) {
        g_prg32_audio.task_stop = true;
        for (int i = 0; i < 20 && g_prg32_audio.task; ++i) {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
    prg32_audio_i2s_stop();
    if (g_prg32_audio.lock) {
        vSemaphoreDelete(g_prg32_audio.lock);
    }
    memset(&g_prg32_audio, 0, sizeof(g_prg32_audio));
}

prg32_audio_mode_t prg32_audio_get_mode(void) {
    if (!g_prg32_audio.initialized) {
        return PRG32_AUDIO_DEFAULT_MODE;
    }
    return g_prg32_audio.config.mode;
}

int prg32_audio_is_ready(void) {
    return g_prg32_audio.initialized ? 1 : 0;
}

void prg32_audio_set_master_volume(uint8_t volume) {
    prg32_audio_lock();
    g_prg32_audio.master_volume = volume;
    prg32_audio_unlock();
}

void prg32_audio_set_channel_volume(uint8_t channel, uint8_t volume) {
    if (channel >= CONFIG_PRG32_AUDIO_MAX_VOICES) {
        return;
    }
    prg32_audio_lock();
    g_prg32_audio.channel_volume[channel] = volume;
    prg32_audio_unlock();
}

void prg32_audio_set_channel_pan(uint8_t channel, int8_t pan) {
    if (channel >= CONFIG_PRG32_AUDIO_MAX_VOICES) {
        return;
    }
    if (pan < PRG32_AUDIO_PAN_LEFT) {
        pan = PRG32_AUDIO_PAN_LEFT;
    }
    if (pan > PRG32_AUDIO_PAN_RIGHT) {
        pan = PRG32_AUDIO_PAN_RIGHT;
    }
    prg32_audio_lock();
    g_prg32_audio.channel_pan[channel] = pan;
    if (channel < g_prg32_audio.max_voices) {
        g_prg32_audio.voices[channel].pan = pan;
    }
    prg32_audio_unlock();
}
