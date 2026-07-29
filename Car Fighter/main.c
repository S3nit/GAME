#include "game.h"
#include "ui.h"

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Road Fighter - Modular Edition");
    SetTargetFPS(60);

    GameContext game;
    InitGame(&game);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // 1. Logic Developer's domain
        UpdateGame(&game, dt);

        // 2. UI Developer's domain
        RenderGame(&game);
    }

    CloseWindow();
    return 0;
}