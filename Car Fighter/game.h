#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>

#define MAX_ENEMIES 5

// --- ENUMS ---
typedef enum { PLAYING, SKIDDING, GAME_OVER } GameState;
typedef enum { ENEMY_NORMAL, ENEMY_FUEL } EnemyType;

// --- CONSTANTS ---
extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;
extern const float LANE_CENTERS[3];

// --- DATA STRUCTURES ---
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

typedef struct {
    GameState currentState;
    Player player;
    Enemy enemies[MAX_ENEMIES];
    float roadOffset;
    float gameSpeed;
    float floatScore;
    float fuel;
} GameContext;

// --- LOGIC FUNCTIONS ---
void InitGame(GameContext *game);
void UpdateGame(GameContext *game, float dt);

#endif // GAME_H