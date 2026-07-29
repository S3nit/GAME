#include "raylib.h"
#include "config.h"
#include "entities.h"
#include "renderer.h"

// Define global constants from config.h
const int SCREEN_WIDTH = 400;
const int SCREEN_HEIGHT = 600;
const float LANE_CENTERS[3] = { 100.0f, 200.0f, 300.0f }; 

const Color COLOR_ASPHALT  = (Color){ 43, 45, 48, 255 };
const Color COLOR_GRASS    = (Color){ 67, 143, 75, 255 };
const Color COLOR_CURB     = (Color){ 200, 200, 200, 255 };
const Color COLOR_PLAYER   = (Color){ 220, 50, 70, 255 };
const Color COLOR_ENEMY    = (Color){ 50, 130, 220, 255 };
const Color COLOR_FUEL_CAR = (Color){ 240, 100, 240, 255 };
const Color COLOR_UI_PANEL = (Color){ 20, 20, 25, 200 };

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Road Fighter - Modular Architecture");
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
        if (dt > 0.05f) dt = 0.05f;

        if (currentState == PLAYING || currentState == SKIDDING) {
            roadOffset += gameSpeed * dt;
            if (roadOffset >= 40.0f) roadOffset = 0.0f;
            
            if (fuel <= 0) {
                fuel = 0;
                currentState = GAME_OVER;
            }

            UpdatePlayer(&player, &gameSpeed, &fuel, &floatScore, &currentState, dt);
            UpdateEnemies(enemies, &player, gameSpeed, &fuel, &floatScore, &currentState, dt);
            
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

        DrawGameScene(player, enemies, roadOffset, gameSpeed, fuel, floatScore, currentState);
    }

    CloseWindow();
    return 0;
}