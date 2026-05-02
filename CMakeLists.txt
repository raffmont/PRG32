cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(PRG32)
set(CART_SOURCES
    ${CMAKE_SOURCE_DIR}/examples/games/pong/graphics/game.S
)