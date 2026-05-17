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

extern int aimCol; // Współrzędna X celownika
extern int aimRow; // Współrzędna Y celownika

extern GameState currentState;

extern int currentShipIndex;
extern int selectedCol, selectedRow;
extern int rotation; 

extern byte playerBoard[8][8];

bool isWithinBounds(int col, int row, int shipIdx, int rot);

void initLogic();
bool canPlaceShip(int col, int row, int shipIdx, int rot);
void placeShip(int col, int row, int shipIdx, int rot);
Point rotatePoint(Point p, int rot); // Dodaj to tutaj, żeby Display.cpp ją widział

#endif // <--- TO MUSI TU BYĆ!