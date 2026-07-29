#include "ui.h"

// --- UI COLOR PALETTE ---
static const Color COLOR_ASPHALT  = (Color){ 43, 45, 48, 255 };
static const Color COLOR_GRASS    = (Color){ 67, 143, 75, 255 };
static const Color COLOR_CURB     = (Color){ 200, 200, 200, 255 };
static const Color COLOR_UI_PANEL = (Color){ 20, 20, 25, 200 };

void RenderGame(const GameContext *game) {
    BeginDrawing();
    ClearBackground(COLOR_ASPHALT);

    // 1. Draw Environment & Road
    DrawRectangle(0, 0, 50, SCREEN_HEIGHT, COLOR_GRASS);
    DrawRectangle(SCREEN_WIDTH - 50, 0, 50, SCREEN_HEIGHT, COLOR_GRASS);
    DrawRectangle(45, 0, 5, SCREEN_HEIGHT, COLOR_CURB);
    DrawRectangle(SCREEN_WIDTH - 50, 0, 5, SCREEN_HEIGHT, COLOR_CURB);

    for (int i = -1; i < SCREEN_HEIGHT / 40 + 2; i++) {
        DrawRectangle(148, i * 40 + game->roadOffset, 4, 20, Fade(WHITE, 0.4f));
        DrawRectangle(248, i * 40 + game->roadOffset, 4, 20, Fade(WHITE, 0.4f));
    }

    // 2. Draw Enemy Cars
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (game->enemies[i].active) {
            DrawRectangleRounded((Rectangle){game->enemies[i].rect.x + 3, game->enemies[i].rect.y + 3, 30, 50}, 0.2f, 4, Fade(BLACK, 0.3f));
            DrawRectangleRounded(game->enemies[i].rect, 0.2f, 4, game->enemies[i].color);

            if (game->enemies[i].isShifting && (int)(GetTime() * 10) % 2 == 0) {
                DrawRectangle(game->enemies[i].rect.x + 5, game->enemies[i].rect.y + 5, 4, 4, YELLOW);
                DrawRectangle(game->enemies[i].rect.x + game->enemies[i].rect.width - 9, game->enemies[i].rect.y + 5, 4, 4, YELLOW);
            }
        }
    }

    // 3. Draw Player Car
    DrawRectangleRounded((Rectangle){game->player.rect.x + 3, game->player.rect.y + 3, 30, 50}, 0.2f, 4, Fade(BLACK, 0.4f));
    if (game->currentState == SKIDDING && (int)(GetTime() * 12) % 2 == 0) {
        DrawRectangleRounded(game->player.rect, 0.2f, 4, WHITE);
    } else {
        DrawRectangleRounded(game->player.rect, 0.2f, 4, game->player.color);
    }

    // 4. Draw HUD / Dashboards
    DrawRectangle(0, 0, SCREEN_WIDTH, 65, COLOR_UI_PANEL);
    DrawLine(0, 65, SCREEN_WIDTH, 65, Fade(WHITE, 0.1f));

    DrawText("SCORE", 20, 15, 10, LIGHTGRAY);
    DrawText(TextFormat("%06i", (int)game->floatScore), 20, 27, 20, WHITE);
    DrawText(TextFormat("SPEED: %03i KM/H", (int)(game->gameSpeed / 2.0f)), 20, 50, 10, ORANGE);

    float maxFuelWidth = 140.0f;
    float currentFuelWidth = (game->fuel / 100.0f) * maxFuelWidth;
    Color fuelColor = game->fuel > 50.0f ? LIME : (game->fuel > 20.0f ? ORANGE : RED);

    DrawText("FUEL", SCREEN_WIDTH - 175, 15, 10, LIGHTGRAY);
    DrawRectangleRounded((Rectangle){ SCREEN_WIDTH - 175, 30, maxFuelWidth, 18 }, 0.5f, 4, Fade(DARKGRAY, 0.5f));
    DrawRectangleRounded((Rectangle){ SCREEN_WIDTH - 175, 30, currentFuelWidth, 18 }, 0.5f, 4, fuelColor);
    DrawRectangleRoundedLines((Rectangle){ SCREEN_WIDTH - 175, 30, maxFuelWidth, 18 }, 0.5f, 4, Fade(WHITE, 0.2f));

    // 5. Game Over Overlay
    if (game->currentState == GAME_OVER) {
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