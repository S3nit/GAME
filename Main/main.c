#include "raylib.h"
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define MAX_ENEMIES 5

// --- ENUMS & CONSTANTS ---
typedef enum { PLAYING, SKIDDING, GAME_OVER } GameState;
typedef enum { ENEMY_NORMAL, ENEMY_FUEL } EnemyType;

const int SCREEN_WIDTH = 400;
const int SCREEN_HEIGHT = 600;
const float LANE_CENTERS[3] = { 100.0f, 200.0f, 300.0f };

// --- COLOR PALETTE ---
const Color COLOR_ASPHALT  = (Color){ 43, 45, 48, 255 };
const Color COLOR_GRASS    = (Color){ 67, 143, 75, 255 };
const Color COLOR_CURB     = (Color){ 200, 200, 200, 255 };
const Color COLOR_PLAYER   = (Color){ 220, 50, 70, 255 };
const Color COLOR_ENEMY    = (Color){ 50, 130, 220, 255 };
const Color COLOR_FUEL_CAR = (Color){ 240, 100, 240, 255 };
const Color COLOR_UI_PANEL = (Color){ 20, 20, 25, 200 };

typedef struct {
    Rectangle rect;
    float speed;
    Color color;
    bool active;
    EnemyType type;
    int currentLane;     
    int targetLane;      
    float laneShiftTimer;
    bool isShifting;
} Enemy;

typedef struct {
    Rectangle rect;
    float velocityX;     
    Color color;
    float skidTimer;     
    int skidDirection;   
} Player;

float SmoothLerp(float start, float end, float amount) {
    return start + amount * (end - start);
}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Road Fighter - Ultra Smooth Physics");
    SetTargetFPS(60);

    Player player = { 
        { SCREEN_WIDTH / 2.0f - 15.0f, SCREEN_HEIGHT - 120.0f, 30.0f, 50.0f }, 
        0.0f, COLOR_PLAYER, 0.0f, 0 
    };
    
    Enemy enemies[MAX_ENEMIES] = { 0 };
    GameState currentState = PLAYING;
    
    float roadOffset = 0.0f;
    float gameSpeed = 200.0f;    
    float floatScore = 0.0f;     
    float fuel = 100.0f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        // Prevent physics blowout if frame rate hiccups
        if (dt > 0.05f) dt = 0.05f;

        // --- UPDATE LOGIC ---
        if (currentState == PLAYING || currentState == SKIDDING) {
            
            roadOffset += gameSpeed * dt;
            if (roadOffset >= 40.0f) roadOffset = 0.0f;
            
            if (fuel <= 0) {
                fuel = 0;
                currentState = GAME_OVER;
            }

            // 2. State: PLAYING
            if (currentState == PLAYING) {
                bool isAccelerating = IsKeyDown(KEY_UP);
                bool isBraking = IsKeyDown(KEY_DOWN);

                // Smooth engine acceleration & braking response curves
                if (isAccelerating) {
                    gameSpeed += 400.0f * dt;  
                } else if (isBraking) {
                    gameSpeed -= 500.0f * dt;  
                } else {
                    if (gameSpeed > 200.0f) gameSpeed -= 180.0f * dt;
                    else if (gameSpeed < 200.0f) gameSpeed += 120.0f * dt; 
                }

                if (gameSpeed > 650.0f) gameSpeed = 650.0f;
                if (gameSpeed < 50.0f) gameSpeed = 50.0f;

                // --- ULTRA-SMOOTH INERTIAL STEERING ---
                float targetVelocityX = 0.0f;
                if (IsKeyDown(KEY_LEFT)) {
                    targetVelocityX = -320.0f;
                }
                if (IsKeyDown(KEY_RIGHT)) {
                    targetVelocityX = 320.0f;
                }

                // Smoothly blend current velocity towards target velocity using exponential damping
                // Higher multiplier = sharper response, lower = heavier/smoother glide
                float steeringResponse = (targetVelocityX == 0.0f) ? 12.0f : 16.0f;
                player.velocityX = SmoothLerp(player.velocityX, targetVelocityX, steeringResponse * dt);

                // Apply velocity to position
                player.rect.x += player.velocityX * dt;

                // Road boundary limits
                if (player.rect.x <= 50) {
                    player.rect.x = 50;
                    player.velocityX = 0.0f;
                } else if (player.rect.x >= SCREEN_WIDTH - 50 - player.rect.width) {
                    player.rect.x = SCREEN_WIDTH - 50 - player.rect.width;
                    player.velocityX = 0.0f;
                }

                // Fuel Consumption with Aerodynamic Drag
                float drainRate = 1.0f; 
                if (isAccelerating) drainRate += 2.5f; 
                drainRate += (gameSpeed * gameSpeed) * 0.000015f; 
                fuel -= drainRate * dt;

                floatScore += (gameSpeed / 100.0f) * 30.0f * dt;
            }
            
            // 3. State: SKIDDING
            if (currentState == SKIDDING) {
                gameSpeed = SmoothLerp(gameSpeed, 100.0f, 3.0f * dt);
                fuel -= 1.0f * dt; 
                player.skidTimer -= dt;
                
                player.velocityX = player.skidDirection * 280.0f;
                player.rect.x += player.velocityX * dt;
                
                if ((player.skidDirection == -1 && IsKeyPressed(KEY_RIGHT)) || 
                    (player.skidDirection == 1 && IsKeyPressed(KEY_LEFT))) {
                    currentState = PLAYING; 
                    player.velocityX = 0.0f; 
                }

                if (player.skidTimer <= 0.0f) currentState = PLAYING; 

                if (player.rect.x <= 50 || player.rect.x >= SCREEN_WIDTH - 50 - player.rect.width) {
                    currentState = GAME_OVER;
                }
            }

            // 4. Enemy Management & Lane Shifting AI
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (!enemies[i].active) {
                    if (GetRandomValue(0, 100) < 2) { 
                        int spawnLane = GetRandomValue(0, 2);
                        
                        bool laneBlocked = false;
                        for (int j = 0; j < MAX_ENEMIES; j++) {
                            if (enemies[j].active && enemies[j].targetLane == spawnLane && enemies[j].rect.y < 120.0f) {
                                laneBlocked = true;
                                break;
                            }
                        }

                        if (!laneBlocked) {
                            enemies[i].active = true;
                            enemies[i].currentLane = spawnLane;
                            enemies[i].targetLane = spawnLane;
                            enemies[i].isShifting = false;
                            enemies[i].laneShiftTimer = (float)GetRandomValue(3, 7);
                            
                            enemies[i].rect = (Rectangle){ LANE_CENTERS[spawnLane] - 15.0f, -60.0f, 30.0f, 50.0f };
                            enemies[i].speed = (float)GetRandomValue(100, 180);
                            
                            if (GetRandomValue(0, 10) > 8) {
                                enemies[i].type = ENEMY_FUEL;
                                enemies[i].color = COLOR_FUEL_CAR;
                            } else {
                                enemies[i].type = ENEMY_NORMAL;
                                enemies[i].color = COLOR_ENEMY;
                            }
                        }
                    }
                } else {
                    enemies[i].rect.y += (gameSpeed - enemies[i].speed) * dt;
                    if (enemies[i].rect.y > SCREEN_HEIGHT) enemies[i].active = false;

                    // Enemy Lane Shifting AI
                    if (!enemies[i].isShifting) {
                        enemies[i].laneShiftTimer -= dt;
                        if (enemies[i].laneShiftTimer <= 0.0f) {
                            int shiftDir = GetRandomValue(0, 1) == 0 ? -1 : 1;
                            int potentialLane = enemies[i].currentLane + shiftDir;

                            if (potentialLane >= 0 && potentialLane <= 2) {
                                bool targetLaneClear = true;
                                for (int j = 0; j < MAX_ENEMIES; j++) {
                                    if (i != j && enemies[j].active) {
                                        if (enemies[j].targetLane == potentialLane && 
                                            fabsf(enemies[i].rect.y - enemies[j].rect.y) < 90.0f) {
                                            targetLaneClear = false;
                                            break;
                                        }
                                    }
                                }

                                if (targetLaneClear) {
                                    enemies[i].targetLane = potentialLane;
                                    enemies[i].isShifting = true;
                                }
                            }
                            enemies[i].laneShiftTimer = (float)GetRandomValue(4, 8);
                        }
                    }

                    // Smoothly glide enemy horizontally
                    float targetX = LANE_CENTERS[enemies[i].targetLane] - (enemies[i].rect.width / 2.0f);
                    enemies[i].rect.x = SmoothLerp(enemies[i].rect.x, targetX, 7.0f * dt);

                    if (fabsf(enemies[i].rect.x - targetX) < 1.0f) {
                        enemies[i].currentLane = enemies[i].targetLane;
                        enemies[i].isShifting = false;
                    }

                    // Collision Detection
                    if (currentState == PLAYING && CheckCollisionRecs(player.rect, enemies[i].rect)) {
                        if (enemies[i].type == ENEMY_FUEL) {
                            fuel += 35.0f; 
                            if (fuel > 100.0f) fuel = 100.0f;
                            enemies[i].active = false;
                            floatScore += 500.0f;
                        } else {
                            currentState = SKIDDING;
                            player.skidTimer = 0.9f; 
                            player.skidDirection = (player.rect.x < enemies[i].rect.x) ? -1 : 1;
                        }
                    }
                }
            }
        } else if (currentState == GAME_OVER) {
            if (IsKeyPressed(KEY_ENTER)) {
                currentState = PLAYING;
                fuel = 100.0f;
                floatScore = 0.0f;
                gameSpeed = 200.0f;
                player.velocityX = 0.0f;
                player.rect.x = SCREEN_WIDTH / 2.0f - 15.0f;
                for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].active = false;
            }
        }

        // --- DRAWING LOGIC ---
        BeginDrawing();
        ClearBackground(COLOR_ASPHALT);

        DrawRectangle(0, 0, 50, SCREEN_HEIGHT, COLOR_GRASS);
        DrawRectangle(SCREEN_WIDTH - 50, 0, 50, SCREEN_HEIGHT, COLOR_GRASS);
        DrawRectangle(45, 0, 5, SCREEN_HEIGHT, COLOR_CURB);
        DrawRectangle(SCREEN_WIDTH - 50, 0, 5, SCREEN_HEIGHT, COLOR_CURB);

        for (int i = -1; i < SCREEN_HEIGHT / 40 + 2; i++) {
            DrawRectangle(148, i * 40 + roadOffset, 4, 20, Fade(WHITE, 0.4f));
            DrawRectangle(248, i * 40 + roadOffset, 4, 20, Fade(WHITE, 0.4f));
        }

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
        
        DrawRectangleRounded((Rectangle){player.rect.x + 3, player.rect.y + 3, 30, 50}, 0.2f, 4, Fade(BLACK, 0.4f));
        if (currentState == SKIDDING && (int)(GetTime() * 12) % 2 == 0) {
            DrawRectangleRounded(player.rect, 0.2f, 4, WHITE);
        } else {
            DrawRectangleRounded(player.rect, 0.2f, 4, player.color);
        }

        // --- UI RENDERING ---
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

    CloseWindow();
    return 0;
}