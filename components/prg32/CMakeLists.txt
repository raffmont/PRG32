idf_component_register(
    SRCS
        "prg32_audio.c"
        "prg32_console.c"
        "prg32_controller.c"
        "prg32_display_ili9341.c"
        "prg32_input.c"
        "prg32_sprite.c"
        "prg32_system.c"
        "prg32_tile.c"
        "prg32_wifi_scores.c"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "../../main"
    PRIV_REQUIRES driver esp_timer esp_wifi esp_http_server esp_http_client esp_event esp_netif nvs_flash json
)
