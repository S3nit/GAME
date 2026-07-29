#include "entities.h"
#include "physics.h"
#include <stdlib.h>
#include <math.h>

void UpdatePlayer(Player *player, float *gameSpeed, float *fuel, float *floatScore, GameState *currentState, float dt) {
    if (*currentState == PLAYING) {
        bool isAccelerating = IsKeyDown(KEY_UP);
        bool isBraking = IsKeyDown(KEY_DOWN);

        if (isAccelerating) {
            *gameSpeed += 400.0f * dt;  
        } else if (isBraking) {
            *gameSpeed -= 500.0f * dt;  
        } else {
            if (*gameSpeed > 200.0f) *gameSpeed -= 180.0f * dt;
            else if (*gameSpeed < 200.0f) *gameSpeed += 120.0f * dt; 
        }

        if (*gameSpeed > 650.0f) *gameSpeed = 650.0f;
        if (*gameSpeed < 50.0f) *gameSpeed = 50.0f;

        float targetVelocityX = 0.0f;
        if (IsKeyDown(KEY_LEFT)) targetVelocityX = -320.0f;
        if (IsKeyDown(KEY_RIGHT)) targetVelocityX = 320.0f;

        float steeringResponse = (targetVelocityX == 0.0f) ? 12.0f : 16.0f;
        player->velocityX = SmoothLerp(player->velocityX, targetVelocityX, steeringResponse * dt);
        player->rect.x += player->velocityX * dt;

        if (player->rect.x <= 50) {
            player->rect.x = 50;
            player->velocityX = 0.0f;
        } else if (player->rect.x >= SCREEN_WIDTH - 50 - player->rect.width) {
            player->rect.x = SCREEN_WIDTH - 50 - player->rect.width;
            player->velocityX = 0.0f;
        }

        float drainRate = 1.0f; 
        if (isAccelerating) drainRate += 2.5f; 
        drainRate += (*gameSpeed * *gameSpeed) * 0.000015f; 
        *fuel -= drainRate * dt;

        *floatScore += (*gameSpeed / 100.0f) * 30.0f * dt;
    }
    else if (*currentState == SKIDDING) {
        *gameSpeed = SmoothLerp(*gameSpeed, 100.0f, 3.0f * dt);
        *fuel -= 1.0f * dt; 
        player->skidTimer -= dt;
        
        player->velocityX = player->skidDirection * 280.0f;
        player->rect.x += player->velocityX * dt;
        
        if ((player->skidDirection == -1 && IsKeyPressed(KEY_RIGHT)) || 
            (player->skidDirection == 1 && IsKeyPressed(KEY_LEFT))) {
            *currentState = PLAYING; 
            player->velocityX = 0.0f; 
        }

        if (player->skidTimer <= 0.0f) *currentState = PLAYING; 

        if (player->rect.x <= 50 || player->rect.x >= SCREEN_WIDTH - 50 - player->rect.width) {
            *currentState = GAME_OVER;
        }
    }
}

void UpdateEnemies(Enemy enemies[], Player *player, float gameSpeed, float *fuel, float *floatScore, GameState *currentState, float dt) {
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

            float targetX = LANE_CENTERS[enemies[i].targetLane] - (enemies[i].rect.width / 2.0f);
            enemies[i].rect.x = SmoothLerp(enemies[i].rect.x, targetX, 7.0f * dt);

            if (fabsf(enemies[i].rect.x - targetX) < 1.0f) {
                enemies[i].currentLane = enemies[i].targetLane;
                enemies[i].isShifting = false;
            }

            if (*currentState == PLAYING && CheckCollisionRecs(player->rect, enemies[i].rect)) {
                if (enemies[i].type == ENEMY_FUEL) {
                    *fuel += 35.0f; 
                    if (*fuel > 100.0f) *fuel = 100.0f;
                    enemies[i].active = false;
                    *floatScore += 500.0f;
                } else {
                    *currentState = SKIDDING;
                    player->skidTimer = 0.9f; 
                    player->skidDirection = (player->rect.x < enemies[i].rect.x) ? -1 : 1;
                }
            }
        }
    }
}