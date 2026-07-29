#include "renderer.h"

void DrawGameScene(Player player, Enemy enemies[], float roadOffset, float gameSpeed, float fuel, float floatScore, GameState currentState) {
    BeginDrawing();
    ClearBackground(COLOR_ASPHALT);

    // Road & Curbs
    DrawRectangle(0, 0, 50, SCREEN_HEIGHT, COLOR_GRASS);
    DrawRectangle(SCREEN_WIDTH - 50, 0, 50, SCREEN_HEIGHT, COLOR_GRASS);
    DrawRectangle(45, 0, 5, SCREEN_HEIGHT, COLOR_CURB);
    DrawRectangle(SCREEN_WIDTH - 50, 0, 5, SCREEN_HEIGHT, COLOR_CURB);

    for (int i = -1; i < SCREEN_HEIGHT / 40 + 2; i++) {
        DrawRectangle(148, i * 40 + roadOffset, 4, 20, Fade(WHITE, 0.4f));
        DrawRectangle(248, i * 40 + roadOffset, 4, 20, Fade(WHITE, 0.4f));
    }

    // Enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            DrawRectangleRounded((Rectangle){enemies[i].rect.x + 3, enemies[i].rect.y + 3, 30, 50}, 0.2f, 4, Fade(BLACK, 0.3f));
            DrawRectangleRounded(enemies[i].rect, 0.2f, 4, enemies[i].color);

            if (enemies[i].isShifting && (int)(GetTime() * 10) % 2 == 0) {
                DrawRectangle(enemies[i].rect.x + 5, enemies[i].rect.y + 5, 4, 4, YELLOW);
                DrawRectangle(enemies[i].rect.x + enemies[i].rect.width - 9, enemies[i].rect.y + 5, 4, 4, YELLOW);
            }
        }
    }
    
    // Player
    DrawRectangleRounded((Rectangle){player.rect.x + 3, player.rect.y + 3, 30, 50}, 0.2f, 4, Fade(BLACK, 0.4f));
    if (currentState == SKIDDING && (int)(GetTime() * 12) % 2 == 0) {
        DrawRectangleRounded(player.rect, 0.2f, 4, WHITE);
    } else {
        DrawRectangleRounded(player.rect, 0.2f, 4, player.color);
    }

    // UI HUD
    DrawRectangle(0, 0, SCREEN_WIDTH, 65, COLOR_UI_PANEL);
    DrawLine(0, 65, SCREEN_WIDTH, 65, Fade(WHITE, 0.1f));

    DrawText("SCORE", 20, 15, 10, LIGHTGRAY);
    DrawText(TextFormat("%06i", (int)floatScore), 20, 27, 20, WHITE);
    DrawText(TextFormat("SPEED: %03i KM/H", (int)(gameSpeed / 2.0f)), 20, 50, 10, ORANGE);

    float maxFuelWidth = 140.0f;
    float currentFuelWidth = (fuel / 100.0f) * maxFuelWidth;
    Color fuelColor = fuel > 50.0f ? LIME : (fuel > 20.0f ? ORANGE : RED);
    
    DrawText("FUEL", SCREEN_WIDTH - 175, 15, 10, LIGHTGRAY);
    DrawRectangleRounded((Rectangle){ SCREEN_WIDTH - 175, 30, maxFuelWidth, 18 }, 0.5f, 4, Fade(DARKGRAY, 0.5f));
    DrawRectangleRounded((Rectangle){ SCREEN_WIDTH - 175, 30, currentFuelWidth, 18 }, 0.5f, 4, fuelColor);
    DrawRectangleRoundedLines((Rectangle){ SCREEN_WIDTH - 175, 30, maxFuelWidth, 18 }, 0.5f, 4, Fade(WHITE, 0.2f));

    // Game Over Overlay
    if (currentState == GAME_OVER) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.7f));
        
        int panelWidth = 280;
        int panelHeight = 120;
        Rectangle goPanel = { (SCREEN_WIDTH - panelWidth) / 2.0f, (SCREEN_HEIGHT - panelHeight) / 2.0f, panelWidth, panelHeight };
        DrawRectangleRounded(goPanel, 0.1f, 10, COLOR_UI_PANEL);
        DrawRectangleRoundedLines(goPanel, 0.1f, 10, Fade(WHITE, 0.2f));

        DrawText("VEHICLE DISABLED", SCREEN_WIDTH / 2 - MeasureText("VEHICLE DISABLED", 20) / 2, SCREEN_HEIGHT / 2 - 30, 20, RED);
        DrawText("PRESS [ENTER] TO RESTART", SCREEN_WIDTH / 2 - MeasureText("PRESS [ENTER] TO RESTART", 14) / 2, SCREEN_HEIGHT / 2 + 15, 14, LIGHTGRAY);
    }

    EndDrawing();
}