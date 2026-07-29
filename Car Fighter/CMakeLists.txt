cmake_minimum_required(VERSION 3.11)
project(RoadFighterGame C)

set(CMAKE_C_STANDARD 99)

# 1. Add executable and modular source files
add_executable(game 
    main.c 
    game.c 
    ui.c
)

# 2. Include current directory for headers (raylib.h, game.h, ui.h)
target_include_directories(game PRIVATE .)

# 3. Link local static library and Windows dependencies
target_link_libraries(game PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/libraylib.a
    opengl32 
    gdi32 
    winmm
)