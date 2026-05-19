#include "Logic.h"
#include "Config.h"

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
    {6, {{0,0}, {1,0}, {2,0}, {1,1}, {2,1}, {3,1}}}, // Zygzak (shipIdx = 0)
    {4, {{0,0}, {-1,0}, {1,0}, {0,1}}},             // T (shipIdx = 1)
    {3, {{0,0}, {1,0}, {2,0}}},                     // Prosty 3-masztowiec (shipIdx = 2)
    {2, {{0,0}, {1,0}}}                             // Linia 2-masztowiec (shipIdx = 3)
};

void initLogic() {
    currentState = PLACING_SHIPS;
    selectedCol = 3;
    selectedRow = 3;
    rotation = 0;
    currentShipIndex = 0;
    aimCol = 3;
    aimRow = 3;
    for(int r=0; r<8; r++) {
        for(int c=0; c<8; c++) {
            playerBoard[r][c] = 0;
            enemyBoard[r][c] = 0;
        }
    }
}

Point rotatePoint(Point p, int rot) {
    if (rot == 1) return {-p.dy, p.dx};
    if (rot == 2) return {-p.dx, -p.dy};
    if (rot == 3) return {p.dy, -p.dx};
    return p;
}

// Rygorystyczna zasada: sprawdzanie 9 pól wokół każdego segmentu
bool isLegalPlacement(int col, int row, int shipIdx, int rot, bool isPlayer) {
    const ShipShape& s = ALL_SHIPS[shipIdx];
    
    for (int i = 0; i < s.size; i++) {
        Point p = rotatePoint(s.modules[i], rot);
        int c = col + p.dx;
        int r = row + p.dy;

        if (c < 0 || c >= 8 || r < 0 || r >= 8) return false;

        // Sprawdzamy otoczenie 3x3 każdego segmentu na odpowiedniej planszy
        for (int dr = -1; dr <= 1; dr++) {
            for (int dc = -1; dc <= 1; dc++) {
                int nr = r + dr; 
                int nc = c + dc;
                if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                    if (isPlayer && playerBoard[nr][nc] != 0) return false;
                    if (!isPlayer && enemyBoard[nr][nc] != 0) return false;
                }
            }
        }
    }
    return true;
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

void placeShip(int col, int row, int shipIdx, int rot, bool isPlayer) {
    const ShipShape& s = ALL_SHIPS[shipIdx];
    for (int i = 0; i < s.size; i++) {
        Point p = rotatePoint(s.modules[i], rot);
        int c = col + p.dx;
        int r = row + p.dy;
        
        // Zapisujemy unikalne ID statku (index + 10) -> Zygzak=10, T=11, 3maszt=12, 2maszt=13
        byte idValue = 10 + shipIdx; 
        
        if (isPlayer) {
            playerBoard[r][c] = idValue; 
        } else {
            enemyBoard[r][c] = idValue; 
        }
    }
}

void placeComputerShips() {
    randomSeed(analogRead(0) + millis());
    bool allShipsPlaced = false;

    while (!allShipsPlaced) {
        for(int r=0; r<8; r++) {
            for(int c=0; c<8; c++) {
                enemyBoard[r][c] = 0;
            }
        }

        allShipsPlaced = true; 

        for (int i = 0; i < SHIP_COUNT; i++) {
            bool placed = false;
            int attempts = 0;

            while (!placed) {
                int randCol = random(0, 8);
                int randRow = random(0, 8);
                int randRot = random(0, 4);

                if (isLegalPlacement(randCol, randRow, i, randRot, false)) { 
                    placeShip(randCol, randRow, i, randRot, false); 
                    placed = true;
                }

                attempts++;
                if (attempts > 100) {
                    allShipsPlaced = false;
                    break; 
                }
            }

            if (!allShipsPlaced) break; 
        }
    }
    Serial.println("Bot pomyslnie i bezpiecznie rozstawil wszystkie statki!");
}

// Funkcja wywoływana, gdy GRACZ strzela w planszę bota (enemyBoard)
bool registerPlayerShot(int x, int y) {
    if (enemyBoard[y][x] >= 10 && enemyBoard[y][x] <= 13) {
        // Trafienie! Dodajemy maskę trafienia (+20), co da wartości 30-33
        enemyBoard[y][x] += 20;
        return true;
    } else if (enemyBoard[y][x] == 0) {
        enemyBoard[y][x] = 1; // Pudło gracza
        return false;
    }
    return false; // Pole już ostrzelane
}

// Prosta (na razie) funkcja strzału bota w planszę gracza
void computerShot() {
    bool shotDone = false;
    while (!shotDone) {
        int r = random(0, 8);
        int c = random(0, 8);
        byte currentCell = playerBoard[r][c];

        // Strzelaj tylko, jeśli pole nie było wcześniej trafione (>=30) ani nie jest pudłem (1)
        if (currentCell != 1 && currentCell < 30) {
            if (currentCell >= 10 && currentCell <= 13) {
                playerBoard[r][c] += 20; // Bot trafia w statek gracza (+20)
                Serial.printf("Bot strzelil w %c%d i TRAFIL!\n", 'A' + c, r + 1);
            } else if (currentCell == 0) {
                playerBoard[r][c] = 1; // Pudło bota
                Serial.printf("Bot strzelil w %c%d i SPUDLOWAL.\n", 'A' + c, r + 1);
            }
            shotDone = true;
        }
    }
}