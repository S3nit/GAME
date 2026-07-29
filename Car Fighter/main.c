#include "raylib.h"
#include "game.h"
#include "ui.h"

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Road Fighter");
    SetTargetFPS(60);

    GameContext game = {0};
    
    game.currentState = MENU_MAIN;
    game.difficultyLevel = 1;
    game.nameLetterCount = 0;
    game.playerName[0] = '\0'; 

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // 1. UPDATE PHASE
        if (game.currentState == PLAYING || game.currentState == SKIDDING || game.currentState == GAME_OVER) {
            UpdateGame(&game, dt);
        } else {
            UpdateMenuLogic(&game);
        }

        // 2. DRAW PHASE
        BeginDrawing();
        
        if (game.currentState == PLAYING || game.currentState == SKIDDING || game.currentState == GAME_OVER) {
            DrawGameplay(&game);
        } else {
            DrawMenuScreen(&game);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}