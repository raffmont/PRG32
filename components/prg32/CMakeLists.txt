set(PRG32_DISPLAY_SRC "prg32_display_ili9341.c")
set(PRG32_PRIV_REQUIRES
    driver
    esp_timer
    esp_wifi
    esp_http_server
    esp_http_client
    esp_event
    esp_netif
    nvs_flash
    json
    spi_flash
)

if(IDF_TARGET STREQUAL "esp32c3")
    list(APPEND PRG32_PRIV_REQUIRES esp_lcd esp_lcd_qemu_rgb)
endif()

if(CONFIG_PRG32_DISPLAY_QEMU_RGB)
    set(PRG32_DISPLAY_SRC "prg32_display_qemu_rgb.c")
endif()

idf_component_register(
    SRCS
        "prg32_audio.c"
        "prg32_cart.c"
        "prg32_console.c"
        "prg32_controller.c"
        ${PRG32_DISPLAY_SRC}
        "prg32_input.c"
        "prg32_sprite.c"
        "prg32_system.c"
        "prg32_tile.c"
        "prg32_wifi_scores.c"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "../../main"
    PRIV_REQUIRES ${PRG32_PRIV_REQUIRES}
)
