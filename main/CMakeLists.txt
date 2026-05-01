idf_component_register(
    SRCS
        "main.c"
        "../games/pong/ascii/game.S"
        "../games/pong/graphics/game.S"
        "../games/breakout/ascii/game.S"
        "../games/breakout/graphics/game.S"
        "../games/space_invaders/ascii/game.S"
        "../games/space_invaders/graphics/game.S"
        "../games/pacman/ascii/game.S"
        "../games/pacman/graphics/game.S"
    REQUIRES urg32
    INCLUDE_DIRS "."
)
