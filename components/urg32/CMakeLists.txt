idf_component_register(
    SRCS
        "urg32_audio.c"
        "urg32_console.c"
        "urg32_controller.c"
        "urg32_display_ili9341.c"
        "urg32_input.c"
        "urg32_sprite.c"
        "urg32_tile.c"
        "urg32_wifi_scores.c"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "../../main"
    PRIV_REQUIRES driver esp_timer esp_wifi esp_http_server esp_event esp_netif nvs_flash json
)
