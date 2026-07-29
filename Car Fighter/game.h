#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>

#define MAX_ENEMIES 5

// --- EXTENDED GAME STATES ---
typedef enum { 
    STATE_INTRO, 
    STATE_MENU, 
    STATE_PLAYING, 
    STATE_SKIDDING, 
    STATE_GAME_OVER 
} GameState;

typedef enum { ENEMY_NORMAL, ENEMY_FUEL } EnemyType;

extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;
extern const float LANE_CENTERS[3];

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

void InitGame(GameContext *game);
void UpdateGame(GameContext *game, float dt);

#endif // GAME_H