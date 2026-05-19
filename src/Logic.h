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

extern byte p1Fleet[8][8];
extern byte p2Fleet[8][8];

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

bool isLegalPlacement(int col, int row, int shipIdx, int rot, bool isPlayer);
void placeShip(int col, int row, int shipIdx, int rot, bool isPlayer);

void placeComputerShips();
void computerShot();
bool registerPlayerShot(int x, int y); 

// --- NOWOŚĆ: Deklaracje funkcji sprawdzania końca gry i zatopień ---
bool checkGameOver(bool checkPlayerBoard);
bool isShipSunk(int shipIdx, bool isPlayerBoard);
void markSunkShipSurroundings(int shipIdx, bool isPlayerBoard);

extern int compAimCol;
extern int compAimRow;

extern int g_trybGraczy;
extern String imieGracza1;
extern String imieGracza2;

extern String winnerName;
extern String loserName;
void reset2PlayerBoards();
void loadPlayer1View();
void savePlayer1View();
void loadPlayer2View();
void savePlayer2View();

#endif // LOGIC_H