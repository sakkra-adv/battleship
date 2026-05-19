#include "Display.h"
#include "Config.h"
#include "Logic.h"

// Deklaracje wewnętrznych funkcji (żeby drawUI widział je poniżej)
void drawVictoryScreen(const char* playerName);
void drawDefeatScreen(const char* playerName);

void drawUI() {
    // --- KROK INTEGRACYJNY: Przechwycenie ekranów końca gry ---
    if (currentState == STATE_VICTORY) {
        drawVictoryScreen("Leszek"); // Wyświetla ekran zwycięstwa
        return; // Przerywa dalsze rysowanie planszy bitewnej
    }
    if (currentState == STATE_DEFEAT) {
        drawDefeatScreen("Leszek");  // Wyświetla ekran porażki
        return; // Przerywa dalsze rysowanie planszy bitewnej
    }

    // --- Reszta Twojego oryginalnego kodu bitwy ---
    M5.Display.startWrite();
    M5.Display.fillScreen(COLOR_BG);
    
    // --- 1. PASEK INFORMACYJNY (GÓRA EKRANU) ---
    M5.Display.setTextSize(1);
    M5.Display.setCursor(5, 3); 
    if (currentState == PLACING_SHIPS) {
        M5.Display.setTextColor(YELLOW);
        M5.Display.print("Setup: Arrows, R=Rotate, C=Confirm");
    } else if (currentState == PLAYER_TURN) {
        M5.Display.setTextColor(COLOR_TEXT);
        M5.Display.print("Aim (e.g. B4) + [SPACE] to fire");
    } else if (currentState == COMPUTER_TURN) {
        M5.Display.setTextColor(RED);
        M5.Display.print("EvilAI is thinking... watch out!");
    }

    // Dynamiczna podmiana nazwy gracza z sarkazmem!
    if (currentState == COMPUTER_TURN) {
        M5.Display.setTextColor(RED);
        M5.Display.setCursor(5, 17); 
        M5.Display.print("Player: EvilAI 😈");
    } else {
        M5.Display.setTextColor(COLOR_PLAYER);
        M5.Display.setCursor(5, 17); 
        M5.Display.print("Player: Leszek");
    }

    // --- 2. PANEL GRACZA / LEWA LEGENDA ---
    M5.Display.drawRect(5, 35, 85, 95, (currentState == COMPUTER_TURN) ? RED : COLOR_PLAYER);
    M5.Display.drawLine(60, 35, 60, 130, (currentState == COMPUTER_TURN) ? RED : COLOR_PLAYER); 
    M5.Display.setTextColor(COLOR_TEXT);
    M5.Display.setCursor(10, 38);
    M5.Display.print("hits:");

    // Dynamiczne zliczanie i rysowanie statusu okrętów
    for(int i = 0; i < SHIP_COUNT; i++) {
        int yPos = 50 + (i * 20);
        int hitCount = 0;
        int targetID = 30 + i; 

        for (int r = 0; r < GRID_SIZE; r++) {
            for (int c = 0; c < GRID_SIZE; c++) {
                if (currentState == COMPUTER_TURN || currentState == PLACING_SHIPS) {
                    if (playerBoard[r][c] == targetID) hitCount++;
                } else {
                    if (enemyBoard[r][c] == targetID) hitCount++;
                }
            }
        }

        for(int b = 0; b < SHIP_SIZES[i]; b++) {
            int xBox = 10 + b * 7;
            M5.Display.fillRect(xBox, yPos, 5, 8, SHIP_COLORS[i]);
            
            if (b < hitCount) {
                M5.Display.drawLine(xBox, yPos, xBox + 4, yPos + 7, BLACK);
                M5.Display.drawLine(xBox + 4, yPos, xBox, yPos + 7, BLACK);
            }
        }
        
        M5.Display.setCursor(70, yPos);
        M5.Display.printf("%d", hitCount); 
    }

    // --- 3. MAPA I LOGIKA WYŚWIETLANIA (PRAWA STRONA) ---
    for (int r = 0; r < GRID_SIZE; r++) {
        M5.Display.setTextColor(COLOR_TEXT);
        M5.Display.setCursor(OFFSET_X - 12, OFFSET_Y + r * CELL_SIZE + 3);
        M5.Display.printf("%d", r + 1);

        for (int c = 0; c < GRID_SIZE; c++) {
            uint16_t cellColor = COLOR_GRID; 
            bool fillCell = false;
            uint16_t fillColor = COLOR_GRID;

            if (currentState == PLACING_SHIPS || currentState == COMPUTER_TURN) {
                byte cellValue = playerBoard[r][c];
                
                if (cellValue == 1) {
                    cellColor = BLUE; 
                    fillColor = BLUE;
                    fillCell = true;
                }
                else if (cellValue >= 10 && cellValue <= 13) {
                    int shipIdx = cellValue - 10;
                    cellColor = SHIP_COLORS[shipIdx]; 
                    fillColor = SHIP_COLORS[shipIdx];
                    fillCell = true;
                }
                else if (cellValue >= 30 && cellValue <= 33) {
                    int shipIdx = cellValue - 30;
                    cellColor = RED; 
                    fillColor = SHIP_COLORS[shipIdx]; 
                    fillCell = true;
                }
                
                if (fillCell) {
                    M5.Display.fillRect(OFFSET_X + c * CELL_SIZE + 2, OFFSET_Y + r * CELL_SIZE + 2, CELL_SIZE - 4, CELL_SIZE - 4, fillColor);
                }
                M5.Display.drawRect(OFFSET_X + c * CELL_SIZE, OFFSET_Y + r * CELL_SIZE, CELL_SIZE, CELL_SIZE, cellColor);

                if (cellValue >= 30 && cellValue <= 33) {
                    M5.Display.drawLine(OFFSET_X + c * CELL_SIZE + 4, OFFSET_Y + r * CELL_SIZE + 4, OFFSET_X + (c + 1) * CELL_SIZE - 5, OFFSET_Y + (r + 1) * CELL_SIZE - 5, RED);
                    M5.Display.drawLine(OFFSET_X + (c + 1) * CELL_SIZE - 5, OFFSET_Y + r * CELL_SIZE + 4, OFFSET_X + c * CELL_SIZE + 4, OFFSET_Y + (r + 1) * CELL_SIZE - 5, RED);
                }
            }
            else if (currentState == PLAYER_TURN) {
                byte cellValue = enemyBoard[r][c];

                if (cellValue == 1) { 
                    cellColor = BLUE;
                    fillColor = BLUE;
                    fillCell = true;
                } 
                else if (cellValue >= 30 && cellValue <= 33) { 
                    int shipIdx = cellValue - 30;
                    fillColor = SHIP_COLORS[shipIdx]; 
                    cellColor = fillColor;
                    fillCell = true;
                }

                if (fillCell) {
                    M5.Display.fillRect(OFFSET_X + c * CELL_SIZE + 2, OFFSET_Y + r * CELL_SIZE + 2, CELL_SIZE - 4, CELL_SIZE - 4, fillColor);
                    M5.Display.drawRect(OFFSET_X + c * CELL_SIZE, OFFSET_Y + r * CELL_SIZE, CELL_SIZE, CELL_SIZE, cellColor);
                    
                    if (cellValue >= 30 && cellValue <= 33) {
                        M5.Display.drawLine(OFFSET_X + c * CELL_SIZE + 4, OFFSET_Y + r * CELL_SIZE + 4, OFFSET_X + (c + 1) * CELL_SIZE - 5, OFFSET_Y + (r + 1) * CELL_SIZE - 5, BLACK);
                        M5.Display.drawLine(OFFSET_X + (c + 1) * CELL_SIZE - 5, OFFSET_Y + r * CELL_SIZE + 4, OFFSET_X + c * CELL_SIZE + 4, OFFSET_Y + (r + 1) * CELL_SIZE - 5, BLACK);
                    }
                } else {
                    M5.Display.drawRect(OFFSET_X + c * CELL_SIZE, OFFSET_Y + r * CELL_SIZE, CELL_SIZE, CELL_SIZE, COLOR_GRID);
                }
            }
        }
    }

    // --- 4. ELEMENTY SPECJALNE (DUCH STATKU / CELOWNIKI) ---
    if (currentState == PLACING_SHIPS) {
        bool legal = isLegalPlacement(selectedCol, selectedRow, currentShipIndex, rotation, true);
        uint16_t ghostColor = legal ? YELLOW : RED;

        if (currentShipIndex < SHIP_COUNT) {
            const ShipShape& s = ALL_SHIPS[currentShipIndex];
            for (int i = 0; i < s.size; i++) {
                Point p = rotatePoint(s.modules[i], rotation);
                int drawC = selectedCol + p.dx;
                int drawR = selectedRow + p.dy;
                
                if (drawC >= 0 && drawC < GRID_SIZE && drawR >= 0 && drawR < GRID_SIZE) {
                    M5.Display.drawRect(OFFSET_X + drawC * CELL_SIZE + 1, 
                                        OFFSET_Y + drawR * CELL_SIZE + 1, 
                                        CELL_SIZE - 2, CELL_SIZE - 2, ghostColor);
                }
            }
        }
    } 
    else if (currentState == PLAYER_TURN) {
        M5.Display.drawRect(OFFSET_X + aimCol * CELL_SIZE - 1, OFFSET_Y - 1, CELL_SIZE + 2, (GRID_SIZE * CELL_SIZE) + 2, 0x7BEF);
        M5.Display.drawRect(OFFSET_X + aimCol * CELL_SIZE + 1, OFFSET_Y + aimRow * CELL_SIZE + 1, CELL_SIZE - 2, CELL_SIZE - 2, WHITE);
        M5.Display.drawRect(OFFSET_X + aimCol * CELL_SIZE + 2, OFFSET_Y + aimRow * CELL_SIZE + 2, CELL_SIZE - 4, CELL_SIZE - 4, YELLOW);
    }
    else if (currentState == COMPUTER_TURN) {
        M5.Display.drawRect(OFFSET_X + compAimCol * CELL_SIZE - 1, OFFSET_Y - 1, CELL_SIZE + 2, (GRID_SIZE * CELL_SIZE) + 2, RED);
        M5.Display.drawRect(OFFSET_X + compAimCol * CELL_SIZE + 1, OFFSET_Y + compAimRow * CELL_SIZE + 1, CELL_SIZE - 2, CELL_SIZE - 2, WHITE);
        M5.Display.drawRect(OFFSET_X + compAimCol * CELL_SIZE + 2, OFFSET_Y + compAimRow * CELL_SIZE + 2, CELL_SIZE - 4, CELL_SIZE - 4, RED);
    }

    // Litery A-H pod mapą
    M5.Display.setTextColor(COLOR_TEXT);
    for (int i = 0; i < GRID_SIZE; i++) {
        M5.Display.setCursor(OFFSET_X + i * CELL_SIZE + 4, OFFSET_Y + (GRID_SIZE * CELL_SIZE) + 4);
        M5.Display.printf("%c", 'A' + i);
    }

    M5.Display.endWrite();
}

// --- FUNKCJE RYSOWANIA EKRANÓW KOŃCOWYCH ---

void drawVictoryScreen(const char* playerName) {
    M5.Display.clear(BLACK);
    M5.Display.drawRect(2, 2, 236, 131, COLOR_PLAYER);
    M5.Display.setTextDatum(TC_DATUM);
    
    M5.Display.setFont(&fonts::DejaVu12);
    M5.Display.setTextColor(COLOR_PLAYER);
    M5.Display.drawString("VICTORY!", 120, 12);
    
    M5.Display.setFont(&fonts::DejaVu9);
    M5.Display.setTextColor(COLOR_TEXT);
    char buffer[40];
    snprintf(buffer, sizeof(buffer), "Captain: %s", playerName);
    M5.Display.drawString(buffer, 120, 37);
    
    M5.Display.setTextColor(GRAY);
    M5.Display.drawString("EvilAI has been destroyed.", 120, 62);
    M5.Display.drawString("Humanity is safe!", 120, 77);
    
    M5.Display.setTextColor(COLOR_PLAYER);
    M5.Display.drawString("[ Press button to restart ]", 120, 107);
}

void drawDefeatScreen(const char* playerName) {
    M5.Display.clear(BLACK);
    M5.Display.drawRect(2, 2, 236, 131, RED);
    M5.Display.setTextDatum(TC_DATUM);
    
    M5.Display.setFont(&fonts::DejaVu12);
    M5.Display.setTextColor(RED);
    M5.Display.drawString("GAME OVER", 120, 12);
    
    M5.Display.setFont(&fonts::DejaVu9);
    M5.Display.setTextColor(COLOR_TEXT);
    char buffer[40];
    snprintf(buffer, sizeof(buffer), "%s - fleet is sinking!", playerName);
    M5.Display.drawString(buffer, 120, 37);
    
    M5.Display.setTextColor(RED);
    M5.Display.drawString("EvilAI: 'Weak algorithm.'", 120, 62);
    M5.Display.drawString("'You went under.'", 120, 77);
    
    M5.Display.setTextColor(GRAY);
    M5.Display.drawString("[ Press button to retry ]", 120, 107);
}