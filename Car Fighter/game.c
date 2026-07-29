#include "game.h"
#include <stdlib.h>
#include <math.h>

const int SCREEN_WIDTH = 400;
const int SCREEN_HEIGHT = 600;
const float LANE_CENTERS[3] = { 100.0f, 200.0f, 300.0f };

static float SmoothLerp(float start, float end, float amount) {
    return start + amount * (end - start);
}

void InitGame(GameContext *game) {
    game->currentState = PLAYING;
    // Player is now 60x100 to prevent gap-sliding
    game->player = (Player){
        .rect = { SCREEN_WIDTH / 2.0f - 30.0f, SCREEN_HEIGHT - 140.0f, 60.0f, 100.0f },
        .velocityX = 0.0f,
        .color = (Color){ 220, 50, 70, 255 },
        .skidTimer = 0.0f,
        .skidDirection = 0
    };
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        game->enemies[i].active = false;
    }
    
    game->roadOffset = 0.0f;
    game->gameSpeed = 200.0f;
    game->floatScore = 0.0f;
    game->fuel = 100.0f;
}

void UpdateGame(GameContext *game, float dt) {
    if (dt > 0.05f) dt = 0.05f; 

    if (game->currentState == PLAYING || game->currentState == SKIDDING) {
        
        game->roadOffset += game->gameSpeed * dt;
        if (game->roadOffset >= 60.0f) game->roadOffset -= 60.0f;
        
        if (game->fuel <= 0) {
            game->fuel = 0;
            game->currentState = GAME_OVER;
        }

        // --- PLAYING STATE ---
        if (game->currentState == PLAYING) {
            bool isAccelerating = IsKeyDown(KEY_UP);
            bool isBraking = IsKeyDown(KEY_DOWN);
            
            if (isAccelerating) {
                game->gameSpeed += 400.0f * dt;
            } else if (isBraking) {
                game->gameSpeed -= 500.0f * dt;
            } else {
                if (game->gameSpeed > 200.0f) game->gameSpeed -= 180.0f * dt;
                else if (game->gameSpeed < 200.0f) game->gameSpeed += 120.0f * dt;
            }
            
            if (game->gameSpeed > 650.0f) game->gameSpeed = 650.0f;
            if (game->gameSpeed < 50.0f) game->gameSpeed = 50.0f;

            float targetVelocityX = 0.0f;
            if (IsKeyDown(KEY_LEFT))  targetVelocityX = -320.0f;
            if (IsKeyDown(KEY_RIGHT)) targetVelocityX = 320.0f;
            
            float steeringResponse = (targetVelocityX == 0.0f) ? 12.0f : 16.0f;
            game->player.velocityX = SmoothLerp(game->player.velocityX, targetVelocityX, steeringResponse * dt);
            game->player.rect.x += game->player.velocityX * dt;

            // Highway Boundaries
            if (game->player.rect.x <= 50) {
                game->player.rect.x = 50;
                game->player.velocityX = 0.0f;
            } else if (game->player.rect.x >= 350 - game->player.rect.width) {
                game->player.rect.x = 350 - game->player.rect.width;
                game->player.velocityX = 0.0f;
            }

            float drainRate = 1.0f;
            if (isAccelerating) drainRate += 2.5f;
            drainRate += (game->gameSpeed * game->gameSpeed) * 0.000015f;
            game->fuel -= drainRate * dt;
            game->floatScore += (game->gameSpeed / 100.0f) * 30.0f * dt;
        }

        // --- SKIDDING STATE ---
        if (game->currentState == SKIDDING) {
            game->gameSpeed = SmoothLerp(game->gameSpeed, 100.0f, 3.0f * dt);
            game->fuel -= 1.0f * dt;
            game->player.skidTimer -= dt;
            game->player.velocityX = game->player.skidDirection * 280.0f;
            game->player.rect.x += game->player.velocityX * dt;
            
            if ((game->player.skidDirection == -1 && IsKeyPressed(KEY_RIGHT)) ||
                (game->player.skidDirection == 1 && IsKeyPressed(KEY_LEFT))) {
                game->currentState = PLAYING;
                game->player.velocityX = 0.0f;
            }
            if (game->player.skidTimer <= 0.0f) game->currentState = PLAYING;
            if (game->player.rect.x <= 50 || game->player.rect.x >= 350 - game->player.rect.width) {
                game->currentState = GAME_OVER;
            }
        }

        // --- ENEMY & ITEM MANAGEMENT ---
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (!game->enemies[i].active) {
                
                int spawnChance = (game->difficultyLevel >= 2) ? 4 : 2;
                
                if (GetRandomValue(0, 100) < spawnChance) {
                    int spawnLane = GetRandomValue(0, 2);
                    bool laneBlocked = false;
                    
                    // The Gap Guarantee Tracker
                    int hazardsInDangerZone = 0;
                    bool occupiedLanes[3] = { false, false, false };
                    
                    for (int j = 0; j < MAX_ENEMIES; j++) {
                        if (game->enemies[j].active) {
                            
                            if (game->enemies[j].targetLane == spawnLane && game->enemies[j].rect.y < 180.0f) {
                                laneBlocked = true;
                            }
                            
                            if (game->enemies[j].type != ENEMY_FUEL && game->enemies[j].rect.y < 350.0f) {
                                if (!occupiedLanes[game->enemies[j].targetLane]) {
                                    occupiedLanes[game->enemies[j].targetLane] = true;
                                    hazardsInDangerZone++;
                                }
                            }
                        }
                    }
                    
                    if (!laneBlocked) {
                        game->enemies[i].active = true;
                        game->enemies[i].currentLane = spawnLane;
                        game->enemies[i].targetLane = spawnLane;
                        game->enemies[i].isShifting = false;
                        
                        int roll = GetRandomValue(0, 100);
                        
                        // Force a fuel can if the path is too blocked
                        if (hazardsInDangerZone >= 2 || roll > 85) {
                            game->enemies[i].type = ENEMY_FUEL;
                            game->enemies[i].speed = 0.0f; 
                            game->enemies[i].color = (Color){ 240, 200, 50, 255 }; 
                            game->enemies[i].rect = (Rectangle){ LANE_CENTERS[spawnLane] - 15.0f, -60.0f, 30.0f, 40.0f };
                            
                        } else if (game->difficultyLevel == 3 && roll < 20) {
                            // Heavy Lorries
                            game->enemies[i].type = ENEMY_LORRY;
                            game->enemies[i].speed = (float)GetRandomValue(70, 110);
                            game->enemies[i].color = (Color){ 100, 140, 110, 255 }; 
                            game->enemies[i].rect = (Rectangle){ LANE_CENTERS[spawnLane] - 30.0f, -180.0f, 60.0f, 180.0f };
                            
                        } else {
                            // Normal Enemy Cars
                            game->enemies[i].type = ENEMY_NORMAL;
                            game->enemies[i].speed = (float)GetRandomValue(100, 180);
                            game->enemies[i].color = (Color){ 50, 130, 220, 255 };
                            game->enemies[i].rect = (Rectangle){ LANE_CENTERS[spawnLane] - 30.0f, -100.0f, 60.0f, 100.0f };
                            game->enemies[i].laneShiftTimer = (float)GetRandomValue(3, 7);
                        }
                    }
                }
            } else {
                
                // Movement Handling
                if (game->enemies[i].type == ENEMY_FUEL) {
                    game->enemies[i].rect.y += game->gameSpeed * dt;
                } else {
                    game->enemies[i].rect.y += (game->gameSpeed - game->enemies[i].speed) * dt;
                }
                
                if (game->enemies[i].rect.y > SCREEN_HEIGHT) game->enemies[i].active = false;

                // Lane Shifting AI (Normal Cars Only)
                if (game->enemies[i].type == ENEMY_NORMAL) {
                    if (!game->enemies[i].isShifting) {
                        game->enemies[i].laneShiftTimer -= dt;
                        if (game->enemies[i].laneShiftTimer <= 0.0f) {
                            int shiftDir = GetRandomValue(0, 1) == 0 ? -1 : 1;
                            int potentialLane = game->enemies[i].currentLane + shiftDir;
                            
                            if (potentialLane >= 0 && potentialLane <= 2) {
                                bool targetLaneClear = true;
                                int distinctLanesOccupied = 0;
                                bool occupied[3] = { false, false, false };
                                
                                occupied[potentialLane] = true; 
                                
                                for (int j = 0; j < MAX_ENEMIES; j++) {
                                    if (i != j && game->enemies[j].active) {
                                        // 1. Prevent crashing into another car in the target lane
                                        if (game->enemies[j].targetLane == potentialLane &&
                                            fabsf(game->enemies[i].rect.y - game->enemies[j].rect.y) < 160.0f) { 
                                            targetLaneClear = false;
                                        }
                                        
                                        // 2. Map all hazard lanes relative to this car's vertical position
                                        if (game->enemies[j].type != ENEMY_FUEL && 
                                            fabsf(game->enemies[i].rect.y - game->enemies[j].rect.y) < 200.0f) {
                                            occupied[game->enemies[j].targetLane] = true;
                                        }
                                    }
                                }
                                
                                for (int l = 0; l < 3; l++) {
                                    if (occupied[l]) distinctLanesOccupied++;
                                }
                                
                                // Cancel shift if it would blockade the entire road
                                if (distinctLanesOccupied == 3) {
                                    targetLaneClear = false;
                                }

                                if (targetLaneClear) {
                                    game->enemies[i].targetLane = potentialLane;
                                    game->enemies[i].isShifting = true;
                                }
                            }
                            game->enemies[i].laneShiftTimer = (float)GetRandomValue(4, 8);
                        }
                    }

                    // Smooth Horizontal Lerp
                    float targetX = LANE_CENTERS[game->enemies[i].targetLane] - (game->enemies[i].rect.width / 2.0f);
                    game->enemies[i].rect.x = SmoothLerp(game->enemies[i].rect.x, targetX, 7.0f * dt);
                    
                    if (fabsf(game->enemies[i].rect.x - targetX) < 1.0f) {
                        game->enemies[i].currentLane = game->enemies[i].targetLane;
                        game->enemies[i].isShifting = false;
                    }
                }

                // Collision Detection
                if (game->currentState == PLAYING && CheckCollisionRecs(game->player.rect, game->enemies[i].rect)) {
                    if (game->enemies[i].type == ENEMY_FUEL) {
                        game->fuel += 35.0f;
                        if (game->fuel > 100.0f) game->fuel = 100.0f;
                        game->enemies[i].active = false;
                        game->floatScore += 500.0f;
                    } else {
                        game->currentState = SKIDDING;
                        game->player.skidTimer = 0.9f;
                        game->player.skidDirection = (game->player.rect.x < game->enemies[i].rect.x) ? -1 : 1;
                    }
                }
            }
        }
    } else if (game->currentState == GAME_OVER) {
        if (IsKeyPressed(KEY_ENTER)) {
            game->currentState = MENU_HIGHSCORES; 
        }
    }
}