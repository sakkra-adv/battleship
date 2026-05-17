#ifndef CONFIG_H
#define CONFIG_H

#include <M5Unified.h>

const int GRID_SIZE = 8;
const int CELL_SIZE = 13; 
const int OFFSET_X = 115; 
const int OFFSET_Y = 12;

const uint16_t COLOR_BG      = BLACK;
const uint16_t COLOR_GRID    = BLUE;
const uint16_t COLOR_SHIP    = GREEN;
const uint16_t COLOR_TEXT    = WHITE;
const uint16_t COLOR_PLAYER  = GREEN;
const uint16_t COLOR_CURSOR  = WHITE;
const uint16_t COLOR_TARGET  = RED;

const int SHIP_COUNT = 4;
const int SHIP_SIZES[] = {6, 4, 3, 2};
const uint16_t SHIP_COLORS[] = {CYAN, MAGENTA, YELLOW, GREEN};

enum GameState {
    PLACING_SHIPS,
    PLAYER_TURN,
    BATTLE_COMPUTER,
    GAME_OVER
};

#endif