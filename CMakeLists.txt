cmake_minimum_required(VERSION 3.11)
project(RoadFighterGame C)

set(CMAKE_C_STANDARD 99)

# 1. Add your game executable and modular source files (removed raylib.c)
add_executable(game 
    main.c 
    entities.c 
    physics.c 
    renderer.c
)

# 2. Include the current directory so CMake finds raylib.h
target_include_directories(game PRIVATE .)

# 3. Link your local libraylib.a archive file along with system libraries
target_link_libraries(game PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/libraylib.a
    opengl32 
    gdi32 
    winmm
)