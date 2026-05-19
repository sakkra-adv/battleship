#ifndef LOGIC_H
#define LOGIC_H

#include "Config.h"
#include <Arduino.h>

struct Point {
    int dx, dy;
};

struct ShipShape {
    int size;
    Point modules[6]; 
};

extern const ShipShape ALL_SHIPS[4];

extern byte playerBoard[8][8];
extern byte enemyBoard[8][8];

extern int aimCol; 
extern int aimRow; 

extern GameState currentState;

extern int currentShipIndex;
extern int selectedCol, selectedRow;
extern int rotation; 

// --- FUNKCJE LOGIKI GRY ---
void initLogic();
Point rotatePoint(Point p, int rot);
bool isWithinBounds(int col, int row, int shipIdx, int rot);

// Poprawione sygnatury uwzględniające rygorystyczne zasady i rozróżnienie gracz/bot
bool isLegalPlacement(int col, int row, int shipIdx, int rot, bool isPlayer);
void placeShip(int col, int row, int shipIdx, int rot, bool isPlayer);

void placeComputerShips();
void computerShot();

// Dodana brakująca deklaracja obsługi strzału gracza dla main.cpp:
bool registerPlayerShot(int x, int y); 

#endif // LOGIC_H