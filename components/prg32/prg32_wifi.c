#include "prg32.h"
#include "prg32_config.h"

#if PRG32_WIFI_ENABLE

#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include <stdbool.h>
#include <string.h>

static const char *TAG = "prg32_wifi";
static bool wifi_started;

static void copy_cstr(char *dst, size_t dst_size, const char *src) {
    if (!dst || dst_size == 0) {
        return;
    }
    if (!src) {
        src = "";
    }
    strncpy(dst, src, dst_size - 1);
    dst[dst_size - 1] = '\0';
}

static void wifi_event(void *arg, esp_event_base_t base, int32_t id, void *data) {
    (void)arg;
    (void)data;
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }
    if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "Wi-Fi connected");
    }
}

void prg32_wifi_scores_init(void) {
    if (wifi_started) {
        return;
    }

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(esp_netif_init());
    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_ERROR_CHECK(err);
    }
#if PRG32_WIFI_STA_ENABLE
    esp_netif_create_default_wifi_sta();
#endif
#if PRG32_WIFI_AP_ENABLE
    esp_netif_create_default_wifi_ap();
#endif

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
#if PRG32_WIFI_STA_ENABLE
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                               ESP_EVENT_ANY_ID,
                                               wifi_event,
                                               NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,
                                               IP_EVENT_STA_GOT_IP,
                                               wifi_event,
                                               NULL));

    wifi_config_t wc = {0};
    copy_cstr((char *)wc.sta.ssid, sizeof(wc.sta.ssid), PRG32_WIFI_SSID);
    copy_cstr((char *)wc.sta.password,
              sizeof(wc.sta.password),
              PRG32_WIFI_PASSWORD);
#endif
#if PRG32_WIFI_AP_ENABLE
    wifi_config_t ap = {0};
    copy_cstr((char *)ap.ap.ssid, sizeof(ap.ap.ssid), PRG32_WIFI_AP_SSID);
    copy_cstr((char *)ap.ap.password,
              sizeof(ap.ap.password),
              PRG32_WIFI_AP_PASSWORD);
    ap.ap.ssid_len = strlen((const char *)ap.ap.ssid);
    ap.ap.channel = PRG32_WIFI_AP_CHANNEL;
    ap.ap.max_connection = PRG32_WIFI_AP_MAX_CONN;
    ap.ap.authmode = strlen((const char *)ap.ap.password) >= 8
        ? WIFI_AUTH_WPA_WPA2_PSK
        : WIFI_AUTH_OPEN;
#endif

#if PRG32_WIFI_STA_ENABLE && PRG32_WIFI_AP_ENABLE
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wc));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap));
#elif PRG32_WIFI_STA_ENABLE
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wc));
#elif PRG32_WIFI_AP_ENABLE
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap));
#endif

    ESP_ERROR_CHECK(esp_wifi_start());
#if PRG32_WIFI_AP_ENABLE
    ESP_LOGI(TAG,
             "Upload AP started: ssid=%s password=%s url=http://192.168.4.1",
             PRG32_WIFI_AP_SSID,
             PRG32_WIFI_AP_PASSWORD);
#endif
    wifi_started = true;
}

#else

void prg32_wifi_scores_init(void) {}

#endif
