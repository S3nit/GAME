#include "ui.h"
#include "raylib.h"

static void DrawTextCentered(const char *text, int y, int fontSize, Color color) {
    int width = MeasureText(text, fontSize);
    DrawText(text, SCREEN_WIDTH / 2 - width / 2, y, fontSize, color);
}

void UpdateMenuLogic(GameContext *game) {
    if (game->currentState == MENU_MAIN) {
        if (IsKeyPressed(KEY_ENTER)) game->currentState = MENU_NAME_INPUT;
        if (IsKeyPressed(KEY_H)) game->currentState = MENU_HIGHSCORES;
    } 
    else if (game->currentState == MENU_NAME_INPUT) {
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125) && (game->nameLetterCount < MAX_NAME_LENGTH - 1)) {
                game->playerName[game->nameLetterCount] = (char)key;
                game->playerName[game->nameLetterCount + 1] = '\0';
                game->nameLetterCount++;
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            if (game->nameLetterCount > 0) {
                game->nameLetterCount--;
                game->playerName[game->nameLetterCount] = '\0';
            }
        }

        if (IsKeyPressed(KEY_ENTER) && game->nameLetterCount > 0) {
            game->currentState = MENU_DIFFICULTY;
        }
    }
    else if (game->currentState == MENU_DIFFICULTY) {
        if (IsKeyPressed(KEY_ONE)) { 
            game->difficultyLevel = 1; 
            InitGame(game); 
        }
        if (IsKeyPressed(KEY_TWO)) { 
            game->difficultyLevel = 2; 
            InitGame(game); 
        }
        if (IsKeyPressed(KEY_THREE)) { 
            game->difficultyLevel = 3; 
            InitGame(game); 
        }
    }
    else if (game->currentState == MENU_HIGHSCORES) {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) {
            game->currentState = MENU_MAIN;
        }
    }
}

void DrawMenuScreen(GameContext *game) {
    ClearBackground(DARKGRAY);

    if (game->currentState == MENU_MAIN) {
        DrawTextCentered("ROAD FIGHTER", 150, 40, RED);
        DrawTextCentered("Press ENTER to Start", 300, 20, LIGHTGRAY);
        DrawTextCentered("Press 'H' for Highscores", 350, 20, LIGHTGRAY);
    } 
    else if (game->currentState == MENU_NAME_INPUT) {
        DrawTextCentered("ENTER PLAYER NAME:", 200, 20, LIGHTGRAY);
        
        DrawRectangle(SCREEN_WIDTH / 2 - 100, 250, 200, 40, BLACK);
        DrawTextCentered(game->playerName, 260, 20, WHITE);
        
        if ((int)(GetTime() * 2.0f) % 2 == 0 && game->nameLetterCount < MAX_NAME_LENGTH - 1) {
            int textWidth = MeasureText(game->playerName, 20);
            DrawText("_", SCREEN_WIDTH / 2 + textWidth / 2 + 5, 260, 20, WHITE);
        }
        
        DrawTextCentered("Press ENTER to continue", 350, 15, GRAY);
    }
    else if (game->currentState == MENU_DIFFICULTY) {
        DrawTextCentered("SELECT DIFFICULTY", 150, 30, WHITE);
        DrawTextCentered("[1] NORMAL (Standard)", 250, 20, GREEN);
        DrawTextCentered("[2] HARD (More Traffic)", 300, 20, YELLOW);
        DrawTextCentered("[3] EXPERT (Heavy Lorries)", 350, 20, RED);
    }
    else if (game->currentState == MENU_HIGHSCORES) {
        DrawTextCentered("HIGHSCORES", 100, 30, GOLD);
        DrawTextCentered("1. GATO - 15000", 200, 20, WHITE);
        DrawTextCentered("2. ALEX - 12000", 240, 20, LIGHTGRAY);
        DrawTextCentered("3. RYAN - 8500", 280, 20, LIGHTGRAY);
        DrawTextCentered("Press ESC or ENTER to return", 500, 15, GRAY);
    }
}

void DrawGameplay(GameContext *game) {
    // 1. Draw the Base Asphalt
    ClearBackground((Color){ 40, 40, 40, 255 });

    // 2. Draw Grass Shoulders
    DrawRectangle(0, 0, 50, SCREEN_HEIGHT, DARKGREEN);
    DrawRectangle(350, 0, 50, SCREEN_HEIGHT, DARKGREEN);

    // 3. Draw Solid White Boundary Lines
    DrawRectangle(50, 0, 8, SCREEN_HEIGHT, RAYWHITE);
    DrawRectangle(342, 0, 8, SCREEN_HEIGHT, RAYWHITE);

    // 4. Draw Scrolling Dashed Lane Dividers
    for (int y = -60; y < SCREEN_HEIGHT; y += 60) {
        DrawRectangle(146, y + (int)game->roadOffset, 8, 30, RAYWHITE);
        DrawRectangle(246, y + (int)game->roadOffset, 8, 30, RAYWHITE);
    }

    // 5. Draw Player
    DrawRectangleRec(game->player.rect, game->player.color);
    // Player details (Windshield & Roof)
    DrawRectangle(game->player.rect.x + 10, game->player.rect.y + 20, game->player.rect.width - 20, 25, BLACK);
    DrawRectangle(game->player.rect.x + 15, game->player.rect.y + 45, game->player.rect.width - 30, 35, DARKGRAY);

    // 6. Draw Enemies/Items
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (game->enemies[i].active) {
            DrawRectangleRec(game->enemies[i].rect, game->enemies[i].color);
            
            if (game->enemies[i].type == ENEMY_LORRY) {
                // Trailer box design
                DrawRectangle(game->enemies[i].rect.x + 5, game->enemies[i].rect.y + 5, 
                              game->enemies[i].rect.width - 10, game->enemies[i].rect.height - 30, LIGHTGRAY);
            } 
            else if (game->enemies[i].type == ENEMY_NORMAL) {
                // Enemy car windshield
                DrawRectangle(game->enemies[i].rect.x + 10, game->enemies[i].rect.y + 20, 
                              game->enemies[i].rect.width - 20, 25, BLACK);
            }
            else if (game->enemies[i].type == ENEMY_FUEL) {
                // Fuel can spout
                DrawRectangle(game->enemies[i].rect.x + 10, game->enemies[i].rect.y - 6, 10, 6, LIGHTGRAY);
            }
        }
    }

    // 7. UI Overlay
    DrawRectangle(0, 0, SCREEN_WIDTH, 40, BLACK);
    DrawText(TextFormat("SCORE: %06i", (int)game->floatScore), 10, 10, 20, WHITE);
    
    DrawText("FUEL:", 220, 10, 20, WHITE);
    Color fuelColor = (game->fuel > 30.0f) ? GREEN : RED;
    DrawRectangle(280, 12, (int)(game->fuel), 15, fuelColor);
    DrawRectangleLines(280, 12, 100, 15, WHITE);

    if (game->currentState == GAME_OVER) {
        DrawRectangle(0, SCREEN_HEIGHT / 2 - 40, SCREEN_WIDTH, 80, Fade(BLACK, 0.8f));
        DrawTextCentered("GAME OVER", SCREEN_HEIGHT / 2 - 20, 40, RED);
        DrawTextCentered("Press ENTER to continue", SCREEN_HEIGHT / 2 + 20, 15, LIGHTGRAY);
    }
}