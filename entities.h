#ifndef ENTITIES_H
#define ENTITIES_H

#include "config.h"

void UpdatePlayer(Player *player, float *gameSpeed, float *fuel, float *floatScore, GameState *currentState, float dt);
void UpdateEnemies(Enemy enemies[], Player *player, float gameSpeed, float *fuel, float *floatScore, GameState *currentState, float dt);

#endif