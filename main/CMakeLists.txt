idf_component_register(
    SRCS
        "main.c"
        "../components/urg32/urg32_console.c"
        "../components/urg32/urg32_display_ili9341.c"
        "../components/urg32/urg32_tile.c"
        "../components/urg32/urg32_sprite.c"
        "../components/urg32/urg32_input.c"
        "../components/urg32/urg32_audio.c"
        "../components/urg32/urg32_controller.c"
        "../components/urg32/urg32_wifi_scores.c"
        "../games/pong/ascii/game.S"
        "../games/pong/graphics/game.S"
        "../games/breakout/ascii/game.S"
        "../games/breakout/graphics/game.S"
        "../games/space_invaders/ascii/game.S"
        "../games/space_invaders/graphics/game.S"
        "../games/pacman/ascii/game.S"
        "../games/pacman/graphics/game.S"
    REQUIRES esp_wifi esp_http_server nvs_flash json driver
    INCLUDE_DIRS "." "../components/urg32/include"
)
