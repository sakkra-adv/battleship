#include "Logic.h"
#include "Config.h"
#include "Display.h"

void playSound(int freq, int durationMs, int type = 0);

byte playerBoard[8][8] = {0};
byte enemyBoard[8][8] = {0};

int aimCol = 3;
int aimRow = 3;

GameState currentState = PLACING_SHIPS;
int selectedCol = 3;
int selectedRow = 3;
int rotation = 0;
int currentShipIndex = 0;
int compAimCol = 3;
int compAimRow = 3;

int g_trybGraczy = 1;
String imieGracza1 = "Kapitan";
String imieGracza2 = "EvilAI";

String winnerName = "";
String loserName = "";

// --- SEJFY PLANSZ DLA 2 GRACZY ---
byte p1Fleet[8][8];      // Statki Gracza 1
byte p1EnemyHits[8][8];  // Tylko strzały Gracza 1 (0, 1, 30-33)
byte p2Fleet[8][8];      // Statki Gracza 2
byte p2EnemyHits[8][8];  // Tylko strzały Gracza 2 (0, 1, 30-33)

void reset2PlayerBoards() {
    memset(p1Fleet, 0, sizeof(p1Fleet));
    memset(p1EnemyHits, 0, sizeof(p1EnemyHits));
    memset(p2Fleet, 0, sizeof(p2Fleet));
    memset(p2EnemyHits, 0, sizeof(p2EnemyHits));
    winnerName = "";
    loserName = "";
}

// Ładowanie świata z perspektywy Gracza 1
void loadPlayer1View() {
    // 1. Lewa strona (Obrona G1): Ładujemy czystą flotę G1
    memcpy(playerBoard, p1Fleet, sizeof(playerBoard));
    
    // 2. Prawa strona (Atak G1): Nakładamy strzały G1 na ukryte statki G2!
    for(int r=0; r<8; r++) {
        for(int c=0; c<8; c++) {
            // Jeśli G1 już tam strzelił (pudło=1, trafienie>=30), pokazujemy to
            if (p1EnemyHits[r][c] != 0) {
                enemyBoard[r][c] = p1EnemyHits[r][c];
            } else {
                // Jeśli nie strzelił, na mapie "wroga" siedzi ukryty statek G2 (0 lub 10-13)
                enemyBoard[r][c] = p2Fleet[r][c];
            }
        }
    }
}

// Zapisywanie stanu po turze Gracza 1
void savePlayer1View() {
    // Zapisujemy stan obrony (mógł dostać obrażenia)
    memcpy(p1Fleet, playerBoard, sizeof(p1Fleet));
    // Zapisujemy tylko historię strzałów podjętych na prawej planszy
    memcpy(p1EnemyHits, enemyBoard, sizeof(p1EnemyHits));
}

// Ładowanie świata z perspektywy Gracza 2
void loadPlayer2View() {
    // 1. Lewa strona (Obrona G2): Ładujemy czystą flotę G2
    memcpy(playerBoard, p2Fleet, sizeof(playerBoard));
    
    // 2. Prawa strona (Atak G2): Nakładamy strzały G2 na ukryte statki G1!
    for(int r=0; r<8; r++) {
        for(int c=0; c<8; c++) {
            if (p2EnemyHits[r][c] != 0) {
                enemyBoard[r][c] = p2EnemyHits[r][c];
            } else {
                enemyBoard[r][c] = p1Fleet[r][c];
            }
        }
    }
}

// Zapisywanie stanu po turze Gracza 2
void savePlayer2View() {
    memcpy(p2Fleet, playerBoard, sizeof(p2Fleet));
    memcpy(p2EnemyHits, enemyBoard, sizeof(p2EnemyHits));
}

// --- ZAAWANSOWANA PAMIĘĆ BEZWZGLĘDNEGO BOTA (EVILAI) ---
struct HitPoint {
    int c, r;
};
HitPoint hitStack[64]; 
int hitStackSize = 0;  

const int dirX[] = {0, 0, -1, 1};
const int dirY[] = {-1, 1, 0, 0};

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
    hitStackSize = 0; 
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

bool isLegalPlacement(int col, int row, int shipIdx, int rot, bool isPlayer) {
    const ShipShape& s = ALL_SHIPS[shipIdx];
    for (int i = 0; i < s.size; i++) {
        Point p = rotatePoint(s.modules[i], rot);
        int c = col + p.dx;
        int r = row + p.dy;
        if (c < 0 || c >= 8 || r < 0 || r >= 8) return false;

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
        byte idValue = 10 + shipIdx; 
        if (isPlayer) playerBoard[r][c] = idValue; 
        else enemyBoard[r][c] = idValue; 
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
    Serial.println("Bot rozstawil statki!");
}

void markSunkShipSurroundings(int shipIdx, bool isPlayerBoard) {
    byte board[8][8];
    for(int r=0; r<8; r++) {
        for(int c=0; c<8; c++) {
            board[r][c] = isPlayerBoard ? playerBoard[r][c] : enemyBoard[r][c];
        }
    }

    byte targetSunkID = 30 + shipIdx; 

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (board[r][c] == targetSunkID) {
                for (int dr = -1; dr <= 1; dr++) {
                    for (int dc = -1; dc <= 1; dc++) {
                        int nr = r + dr;
                        int nc = c + dc;
                        if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                            if (isPlayerBoard && playerBoard[nr][nc] == 0) {
                                playerBoard[nr][nc] = 1; 
                            }
                            if (!isPlayerBoard && enemyBoard[nr][nc] == 0) {
                                enemyBoard[nr][nc] = 1; 
                            }
                        }
                    }
                }
            }
        }
    }
}

bool isShipSunk(int shipIdx, bool isPlayerBoard) {
    byte targetAliveID = 10 + shipIdx; 
    
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (isPlayerBoard) {
                if (playerBoard[r][c] == targetAliveID) return false; 
            } else {
                if (enemyBoard[r][c] == targetAliveID) return false; 
            }
        }
    }
    return true; 
}

bool registerPlayerShot(int x, int y) {
    if (enemyBoard[y][x] >= 10 && enemyBoard[y][x] <= 13) {
        int shipIdx = enemyBoard[y][x] - 10;
        enemyBoard[y][x] += 20; // Trafienie (30-33)
        
        if (isShipSunk(shipIdx, false)) {
            Serial.printf("Zatopiles statek bota o indeksie %d!\n", shipIdx);
            
            // --- BOHATERSKIE FANFARY GRACZA ---
            // Radosne, czyste retro nutki obwieszczające sukces
            playSound(523, 100); // C5
            playSound(659, 100); // E5
            playSound(784, 100); // G5
            playSound(1047, 300); // C6 (Zwycięski akcent!)
            
            markSunkShipSurroundings(shipIdx, false); 
        }

        if (checkGameOver(false)) { 
            currentState = STATE_VICTORY;
            Serial.println("Koniec Gry!");
        }

        return true;
    } else if (enemyBoard[y][x] == 0) {
        enemyBoard[y][x] = 1; 
        return false;
    }
    return false; 
}

void computerShot() {
    // 1. FAZA MYŚLENIA / PRZESZUKIWANIA ("Bi Bu Bi Bu")
    for (int i = 0; i < 6; i++) {
        compAimCol = random(0, 8);
        compAimRow = random(0, 8);
        drawUI(); 
        
        // Generujemy naprzemienne wysokie i niskie piknięcia superkomputera
        if (i % 2 == 0) {
            playSound(900, 40); // Wysokie "Bi"
        } else {
            playSound(600, 40); // Niższe "Bu"
        }
        
        delay(140); // Skrócony delay, bo playSound już ma małe opóźnienie
    }

    int targetRow = -1;
    int targetCol = -1;
    bool shotFound = false;

    // 2. LOGIKA WYBORU CELU PRZEZ AI
    while (hitStackSize > 0 && !shotFound) {
        HitPoint lastHit = hitStack[hitStackSize - 1];
        int startDir = random(0, 4);
        for (int i = 0; i < 4; i++) {
            int dir = (startDir + i) % 4;
            int nextC = lastHit.c + dirX[dir];
            int nextR = lastHit.r + dirY[dir];

            if (nextC >= 0 && nextC < 8 && nextR >= 0 && nextR < 8) {
                byte cell = playerBoard[nextR][nextC];
                if (cell != 1 && cell < 30) { 
                    targetCol = nextC;
                    targetRow = nextR;
                    shotFound = true;
                    break;
                }
            }
        }
        if (!shotFound) {
            hitStackSize--; 
        }
    }

    if (!shotFound) {
        while (!shotFound) {
            int r = random(0, 8);
            int c = random(0, 8);
            if (playerBoard[r][c] != 1 && playerBoard[r][c] < 30) {
                targetRow = r;
                targetCol = c;
                shotFound = true;
            }
        }
    }

    // Najechanie na ostateczny cel i zatwierdzenie ("Piiip!")
    compAimCol = targetCol;
    compAimRow = targetRow;
    drawUI();
    playSound(1200, 100); // Dźwięk namierzenia celu
    delay(400); 

    // 3. ODPALENIE TORPEDY PRZEZ BOT-A
    playSound(800, 50, 1);
    playSound(400, 80, 1);

    byte finalCell = playerBoard[targetRow][targetCol];

    if (finalCell >= 10 && finalCell <= 13) {
        int shipIdx = finalCell - 10;
        playerBoard[targetRow][targetCol] += 20; // TRAFIENIE!
        Serial.printf("EvilAI strzelil w %c%d i TRAFIL!\n", 'A' + targetCol, targetRow + 1);
        
        // Efekt wybuchu w Twoją flotę (brutalny chiptune)
        playSound(700, 80, 1);
        playSound(250, 150, 1);

        if (hitStackSize < 64) {
            hitStack[hitStackSize] = {targetCol, targetRow};
            hitStackSize++;
        }

        if (isShipSunk(shipIdx, true)) {
            Serial.printf("EvilAI zatopil Twoj statek %d!\n", shipIdx);
            
            // --- ZŁOWIESZCZE FANFARY EVILAI ---
            // Krótka, triumfalna i agresywna sekwencja tonów bota
            playSound(440, 100, 1); // A4
            playSound(554, 100, 1); // C#5
            playSound(659, 100, 1); // E5
            playSound(880, 300, 1); // A5 (Triumf!)
            
            markSunkShipSurroundings(shipIdx, true);
            hitStackSize = 0; 
        }

        if (checkGameOver(true)) { 
            currentState = STATE_DEFEAT;
            Serial.println("Koniec Gry! EvilAI wygral.");
        }
    } 
    else if (finalCell == 0) {
        playerBoard[targetRow][targetCol] = 1; // PUDŁO!
        Serial.printf("EvilAI strzelil w %c%d i SPUDLOWAL.\n", 'A' + targetCol, targetRow + 1);
        
        // Smutne, niskie "plum" w wykonaniu bota
        playSound(130, 200);
    }

    drawUI();
    delay(1000); 
}

bool checkGameOver(bool checkPlayerBoard) {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            byte cell = checkPlayerBoard ? playerBoard[r][c] : enemyBoard[r][c];
            if (cell >= 10 && cell <= 13) {
                return false; 
            }
        }
    }
    return true; 
}