#include "Display.h"
#include "Config.h"
#include "Logic.h"

// --- ZMIENNE ZEWNĘTRZNE (EXTERN) ---
extern String imieGracza1;
extern String imieGracza2;
extern GameState currentState;
extern String winnerName;
extern String loserName;
extern int g_trybGraczy;

// Deklaracje wewnętrznych funkcji
void drawVictoryScreen(const char* msg);
void drawDefeatScreen(const char* msg);

void drawUI() {
    // --- 1. PRZECHWYCENIE EKRANÓW KOŃCA GRY ---
    if (currentState == STATE_VICTORY) {
        drawVictoryScreen(""); 
        return; 
    }
    if (currentState == STATE_DEFEAT) {
        drawDefeatScreen("");  
        return; 
    }

    M5.Display.startWrite();
    M5.Display.fillScreen(COLOR_BG);
    
    // --- 2. PASEK INFORMACYJNY (GÓRA EKRANU) ---
    M5.Display.setTextSize(1);
    M5.Display.setCursor(5, 3); 
    
    // Sprawdzamy stan układania statków (Zaktualizowana legenda na SPACJĘ)
    if (currentState == PLACING_SHIPS || currentState == PLACING_P1 || currentState == PLACING_P2) {
        M5.Display.setTextColor(YELLOW);
        M5.Display.print("Move: A S D E, R=Rot, SPACE=Ok");
    } 
    // Sprawdzamy stan strzelania
    else if (currentState == PLAYER_TURN || currentState == PLAYER1_TURN || currentState == PLAYER2_TURN) {
        M5.Display.setTextColor(COLOR_TEXT);
        M5.Display.print("Aim: A S D E + [SPACE] to fire");
    } 
    else if (currentState == COMPUTER_TURN) {
        M5.Display.setTextColor(RED);
        M5.Display.print("EvilAI is thinking... watch out!");
    }

    // Dynamiczny wybór aktywnego imienia do paska "Player: ..."
    M5.Display.setCursor(5, 17); 
    if (currentState == COMPUTER_TURN) {
        M5.Display.setTextColor(RED);
        M5.Display.print("Player: EvilAI 😈");
    } else {
        M5.Display.setTextColor(COLOR_PLAYER);
        M5.Display.print("Player: ");
        if (g_trybGraczy == 2 && (currentState == PLACING_P2 || currentState == PLAYER2_TURN)) {
            M5.Display.print(imieGracza2.c_str());
        } else {
            M5.Display.print(imieGracza1.c_str());
        }
    }

    // --- 3. PANEL STATUSU OKRĘTÓW (LEWA STRONA) ---
    M5.Display.drawRect(5, 35, 85, 95, (currentState == COMPUTER_TURN) ? RED : COLOR_PLAYER);
    M5.Display.drawLine(60, 35, 60, 130, (currentState == COMPUTER_TURN) ? RED : COLOR_PLAYER); 
    M5.Display.setTextColor(COLOR_TEXT);
    M5.Display.setCursor(10, 38);
    M5.Display.print("hits:"); // Nazwa panelu trafień

    // Dynamiczne rysowanie trafień
    for(int i = 0; i < SHIP_COUNT; i++) {
        int yPos = 50 + (i * 20);
        int hitCount = 0;
        int targetID = 30 + i; 

        for (int r = 0; r < GRID_SIZE; r++) {
            for (int c = 0; c < GRID_SIZE; c++) {
                if (currentState == COMPUTER_TURN || currentState == PLACING_SHIPS || currentState == PLACING_P1 || currentState == PLACING_P2) {
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

    // --- 4. RYSOWANIE MAPY BITWY (PRAWA STRONA) ---
    for (int r = 0; r < GRID_SIZE; r++) {
        M5.Display.setTextColor(COLOR_TEXT);
        M5.Display.setCursor(OFFSET_X - 12, OFFSET_Y + r * CELL_SIZE + 3);
        M5.Display.printf("%d", r + 1);

        for (int c = 0; c < GRID_SIZE; c++) {
            uint16_t cellColor = COLOR_GRID; 
            bool fillCell = false;
            uint16_t fillColor = COLOR_GRID;

            if (currentState == PLACING_SHIPS || currentState == PLACING_P1 || currentState == PLACING_P2 || currentState == COMPUTER_TURN) {
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
            else if (currentState == PLAYER_TURN || currentState == PLAYER1_TURN || currentState == PLAYER2_TURN) {
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

    // --- 5. SPECJALNE NAKŁADKI (DUCH STATKU / CELOWNIKI) ---
    if (currentState == PLACING_SHIPS || currentState == PLACING_P1 || currentState == PLACING_P2) {
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
    else if (currentState == PLAYER_TURN || currentState == PLAYER1_TURN || currentState == PLAYER2_TURN) {
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

// --- FUNKCJE RYSOWANIA EKRANÓW KOŃCOWYCH (Angielskie teksty i instrukcje) ---

void drawVictoryScreen(const char* msg) {
    M5.Display.fillScreen(BLACK);
    
    // Zwycięstwo - rozmiar 2
    M5.Display.setTextColor(GREEN);
    M5.Display.setTextSize(2);
    M5.Display.setTextDatum(CC_DATUM);
    M5.Display.drawString("VICTORY!", 120, 25);
    
    M5.Display.setTextColor(WHITE);
    M5.Display.setTextSize(1);
    
    char buforTekstu[64];
    if (g_trybGraczy == 2) {
        snprintf(buforTekstu, sizeof(buforTekstu), "%s defeated %s!", winnerName.c_str(), loserName.c_str());
        M5.Display.drawString(buforTekstu, 120, 65);
    } else {
        snprintf(buforTekstu, sizeof(buforTekstu), "Congratulations %s!", imieGracza1.c_str());
        M5.Display.drawString(buforTekstu, 120, 65);
    }
    
    M5.Display.setTextColor(0x7BEF); 
    M5.Display.setTextSize(1);
    M5.Display.drawString("[ SPACE = Play Again ]", 120, 105);
}

void drawDefeatScreen(const char* msg) {
    M5.Display.fillScreen(BLACK);
    
    // Porażka - rozmiar 2
    M5.Display.setTextColor(RED);
    M5.Display.setTextSize(2);
    M5.Display.setTextDatum(CC_DATUM);
    M5.Display.drawString("DEFEAT!", 120, 25);
    
    M5.Display.setTextColor(WHITE);
    M5.Display.setTextSize(1);
    
    char buforTekstu[64];
    snprintf(buforTekstu, sizeof(buforTekstu), "%s - fleet destroyed...", imieGracza1.c_str());
    M5.Display.drawString(buforTekstu, 120, 65);
    
    M5.Display.setTextColor(0x7BEF); 
    M5.Display.setTextSize(1);
    M5.Display.drawString("[ SPACE = Try Again ]", 120, 105);
}

// Funkcja generująca retro dźwięki
void playSound(int freq, int durationMs, int type) {
    if (type == 1) {
        M5.Speaker.tone(freq, durationMs, 0, true); 
    } else {
        M5.Speaker.tone(freq, durationMs); 
    }
    delay(durationMs); 
}