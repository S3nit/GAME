#include "raylib.h"
#include <stdlib.h>
#include <stdbool.h>
#define MAX_ENEMIES 6

// --- ENUMS for Better Architecture ---
// These explicitly define what state the game and the entities are in.
typedef enum { PLAYING, SKIDDING, GAME_OVER } GameState;
typedef enum { ENEMY_NORMAL, ENEMY_FUEL } EnemyType;

typedef struct {
    Rectangle rect;
    float speed;
    Color color;
    bool active;
    EnemyType type;
} Enemy;

typedef struct {
    Rectangle rect;
    float speed;
    Color color;
    int skidDirection; // -1 for sliding left, 1 for sliding right
} Player;

int main(void) {
    const int screenWidth = 400;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Road Fighter - Advanced Mechanics");

    Player player = { { screenWidth / 2.0f - 15, screenHeight - 120, 30, 50 }, 300.0f, RED, 0 };
    Enemy enemies[MAX_ENEMIES] = { 0 };

    GameState currentState = PLAYING;
    
    float roadOffset = 0.0f;
    float gameSpeed = 200.0f;
    int score = 0;
    float fuel = 100.0f; // Starts at 100%

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // --- UPDATE LOGIC ---
        if (currentState == PLAYING || currentState == SKIDDING) {
            
            // 1. Road scrolling & Fuel drain
            roadOffset += gameSpeed * dt;
            if (roadOffset >= 40.0f) roadOffset = 0.0f;
            
            fuel -= 3.0f * dt; // Constant fuel drain
            if (fuel <= 0) {
                fuel = 0;
                currentState = GAME_OVER;
            }

            // 2. State: PLAYING (Normal Controls)
            if (currentState == PLAYING) {
                if (IsKeyDown(KEY_LEFT) && player.rect.x > 50) {
                    player.rect.x -= player.speed * dt;
                }
                if (IsKeyDown(KEY_RIGHT) && player.rect.x < screenWidth - 50 - player.rect.width) {
                    player.rect.x += player.speed * dt;
                }
                
                if (IsKeyDown(KEY_UP)) {
                    gameSpeed = 500.0f; // High Gear
                    fuel -= 2.0f * dt;  // Burns fuel faster
                    score += 2;
                } else {
                    gameSpeed = 250.0f; // Low Gear
                    score += 1;
                }
            }
            
            // 3. State: SKIDDING (Loss of Control)
            if (currentState == SKIDDING) {
                gameSpeed = 150.0f; // Drastic speed loss while sliding
                player.rect.x += player.skidDirection * 200.0f * dt; // Slide sideways
                
                // Counter-steer shortcut to recover
                if ((player.skidDirection == -1 && IsKeyPressed(KEY_RIGHT)) || 
                    (player.skidDirection == 1 && IsKeyPressed(KEY_LEFT))) {
                    currentState = PLAYING; 
                }

                // If you slide into the grass, you explode
                if (player.rect.x <= 50 || player.rect.x >= screenWidth - 50 - player.rect.width) {
                    currentState = GAME_OVER;
                }
            }

            // 4. Enemy Spawning & Collision
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (!enemies[i].active) {
                    if (GetRandomValue(0, 100) < 2) { 
                        enemies[i].active = true;
                        enemies[i].rect = (Rectangle){ (float)GetRandomValue(50, screenWidth - 80), -50.0f, 30.0f, 50.0f };
                        enemies[i].speed = (float)GetRandomValue(100, 150);
                        
                        // 20% chance to spawn a Fuel Car
                        if (GetRandomValue(0, 10) > 8) {
                            enemies[i].type = ENEMY_FUEL;
                            enemies[i].color = MAGENTA; // Fuel cars are bright pink/magenta
                        } else {
                            enemies[i].type = ENEMY_NORMAL;
                            enemies[i].color = BLUE; // Normal hazard cars
                        }
                    }
                } else {
                    enemies[i].rect.y += (gameSpeed - enemies[i].speed) * dt;
                    if (enemies[i].rect.y > screenHeight) enemies[i].active = false;

                    // Collision Detection (Only check if we aren't already skidding)
                    if (currentState == PLAYING && CheckCollisionRecs(player.rect, enemies[i].rect)) {
                        if (enemies[i].type == ENEMY_FUEL) {
                            fuel += 20.0f; // Reward: Fill tank
                            if (fuel > 100.0f) fuel = 100.0f;
                            enemies[i].active = false; // "Consume" the fuel car
                            score += 500;
                        } else {
                            // Hit a normal car: Trigger Skid!
                            currentState = SKIDDING;
                            // Determine which way the player slides based on impact angle
                            player.skidDirection = (player.rect.x < enemies[i].rect.x) ? -1 : 1;
                        }
                    }
                }
            }
        } else if (currentState == GAME_OVER) {
            // Restart shortcut
            if (IsKeyPressed(KEY_ENTER)) {
                currentState = PLAYING;
                fuel = 100.0f;
                score = 0;
                player.rect.x = screenWidth / 2.0f - 15;
                for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].active = false;
            }
        }

        // --- DRAWING LOGIC ---
        BeginDrawing();
        ClearBackground(DARKGRAY);

        DrawRectangle(0, 0, 50, screenHeight, DARKGREEN);
        DrawRectangle(screenWidth - 50, 0, 50, screenHeight, DARKGREEN);

        for (int i = -1; i < screenHeight / 40 + 2; i++) {
            DrawRectangle(screenWidth / 2 - 5, i * 40 + roadOffset, 10, 20, RAYWHITE);
        }

        // Draw Entities
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active) DrawRectangleRec(enemies[i].rect, enemies[i].color);
        }
        
        // Draw Player (Flash yellow if skidding to indicate danger)
        if (currentState == SKIDDING && (int)(GetTime() * 10) % 2 == 0) {
            DrawRectangleRec(player.rect, YELLOW);
        } else {
            DrawRectangleRec(player.rect, player.color);
        }

        // Draw UI (Score and Fuel Bar)
        DrawText(TextFormat("SCORE: %06i", score), 60, 10, 20, YELLOW);
        
        // Dynamic Fuel Bar (Turns red when below 25%)
        DrawRectangle(60, 40, 100, 10, BLACK);
        DrawRectangle(60, 40, (int)fuel, 10, fuel < 25.0f ? RED : GREEN);
        DrawText("FUEL", 60, 55, 10, WHITE);

        if (currentState == GAME_OVER) {
            DrawRectangle(0, screenHeight / 2 - 40, screenWidth, 80, Fade(BLACK, 0.8f));
            DrawText("CRASHED! OUT OF FUEL!", screenWidth / 2 - 120, screenHeight / 2 - 20, 20, RED);
            DrawText("PRESS ENTER TO RESTART", screenWidth / 2 - 110, screenHeight / 2 + 10, 15, WHITE);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}