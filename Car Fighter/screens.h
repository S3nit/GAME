#ifndef SCREENS_H
#define SCREENS_H

#include "game.h"

// Menu Option Indices
typedef enum {
    MENU_START = 0,
    MENU_CONTROLS,
    MENU_EXIT,
    MENU_OPTION_COUNT
} MenuOption;

// Screen Rendering Prototypes
void RenderIntroScreen(float roadOffset);
void RenderMenuScreen(int selectedOption, bool showControls, float roadOffset);

#endif // SCREENS_H