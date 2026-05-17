#include "Logic.h"
#include "Config.h"

// Inicjalizacja pustych plansz na start gry
byte playerBoard[8][8] = {0};
byte enemyBoard[8][8] = {0};

int aimCol = 3;
int aimRow = 3;

GameState currentState = PLACING_SHIPS;
int selectedCol = 3;
int selectedRow = 3;
int rotation = 0;
int currentShipIndex = 0;

const ShipShape ALL_SHIPS[4] = {
    {6, {{0,0}, {1,0}, {2,0}, {1,1}, {2,1}, {3,1}}}, // Zygzak
    {4, {{0,0}, {-1,0}, {1,0}, {0,1}}},             // T
    {3, {{0,0}, {1,0}, {2,0}}},                     // Linia (Prosty trzymasztowiec!)
    {2, {{0,0}, {1,0}}}                             // Linia
};

void initLogic() {
    currentState = PLACING_SHIPS;
    selectedCol = 0;
    selectedRow = 0;
    rotation = 0;
    currentShipIndex = 0;
    
    // Ustawiamy celownik na środek planszy wroga na start
    aimCol = 3;
    aimRow = 3;

    for(int r=0; r<8; r++) {
        for(int c=0; c<8; c++) {
            playerBoard[r][c] = 0;
            enemyBoard[r][c] = 0; // Czyścimy też planszę bota przy starcie
        }
    }
}

Point rotatePoint(Point p, int rot) {
    if (rot == 1) return {-p.dy, p.dx};
    if (rot == 2) return {-p.dx, -p.dy};
    if (rot == 3) return {p.dy, -p.dx};
    return p;
}

bool isWithinBounds(int col, int row, int shipIdx, int rot) {
    const ShipShape& s = ALL_SHIPS[shipIdx];
    for (int i = 0; i < s.size; i++) {
        Point p = rotatePoint(s.modules[i], rot);
        int c = col + p.dx;
        int r = row + p.dy;
        if (c < 0 || c >= 8 || r < 0 || r >= 8) return false;
    }
    return true;
}

bool canPlaceShip(int col, int row, int shipIdx, int rot) {
    const ShipShape& s = ALL_SHIPS[shipIdx];
    for (int i = 0; i < s.size; i++) {
        Point p = rotatePoint(s.modules[i], rot);
        int c = col + p.dx;
        int r = row + p.dy;

        // 1. Czy nie wychodzi poza mapę
        if (c < 0 || c >= 8 || r < 0 || r >= 8) return false;

        // 2. Czy pole jest WOLNE (musi być równe 0)
        if (playerBoard[r][c] != 0) return false;
    }
    return true;
}

void placeShip(int col, int row, int shipIdx, int rot) {
    const ShipShape& s = ALL_SHIPS[shipIdx];
    for (int i = 0; i < s.size; i++) {
        Point p = rotatePoint(s.modules[i], rot);
        int c = col + p.dx;
        int r = row + p.dy;
        playerBoard[r][c] = 1;
        for (int dr = -1; dr <= 1; dr++) {
            for (int dc = -1; dc <= 1; dc++) {
                int nr = r + dr; int nc = c + dc;
                if (nr >= 0 && nr < GRID_SIZE && nc >= 0 && nc < GRID_SIZE) {
                    if (playerBoard[nr][nc] == 0) playerBoard[nr][nc] = 4;
                }
            }
        }
    }
}