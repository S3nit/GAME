#ifndef CONFIG_H
#define CONFIG_H

#include "raylib.h"

#define MAX_ENEMIES 6

typedef enum { PLAYING, SKIDDING, GAME_OVER } GameState;
typedef enum { ENEMY_NORMAL, ENEMY_FUEL } EnemyType;

extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;
extern const float LANE_CENTERS[3];

// Color Palette
extern const Color COLOR_ASPHALT;
extern const Color COLOR_GRASS;
extern const Color COLOR_CURB;
extern const Color COLOR_PLAYER;
extern const Color COLOR_ENEMY;
extern const Color COLOR_FUEL_CAR;
extern const Color COLOR_UI_PANEL;

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

#endif