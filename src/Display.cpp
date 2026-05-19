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
    
    // Sprawdzamy stan układania statków (dla 1P oraz obu graczy w 2P)
    if (currentState == PLACING_SHIPS || currentState == PLACING_P1 || currentState == PLACING_P2) {
        M5.Display.setTextColor(YELLOW);
        M5.Display.print("Setup: Arrows, R=Rotate, C=Confirm");
    } 
    // Sprawdzamy stan strzelania (dla 1P oaz obu graczy w 2P)
    else if (currentState == PLAYER_TURN || currentState == PLAYER1_TURN || currentState == PLAYER2_TURN) {
        M5.Display.setTextColor(COLOR_TEXT);
        M5.Display.print("Aim (e.g. B4) + [SPACE] to fire");
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
        // Jeśli gra 2 gracz i jest jego faza układania lub strzelania -> dajemy Imię 2
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
    M5.Display.print("hits:");

    // Dynamiczne rysowanie trafień
    for(int i = 0; i < SHIP_COUNT; i++) {
        int yPos = 50 + (i * 20);
        int hitCount = 0;
        int targetID = 30 + i; 

        for (int r = 0; r < GRID_SIZE; r++) {
            for (int c = 0; c < GRID_SIZE; c++) {
                // W tych stanach sprawdzamy naszą własną planszę (obronę)
                if (currentState == COMPUTER_TURN || currentState == PLACING_SHIPS || currentState == PLACING_P1 || currentState == PLACING_P2) {
                    if (playerBoard[r][c] == targetID) hitCount++;
                } else {
                    // W czasie naszej tury sprawdzamy planszę atakowaną
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

            // WIDOK WŁASNEJ PLANSZY (Ustawianie / Tura Przeciwnika)
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
            // WIDOK CELOWANIA DO WROGA (Tura Gracza 1 lub Gracza 2)
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
    
    // Rysowanie ducha rozstawianego statku (Wspólne dla 1P i obu w 2P)
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
    // Celownik gracza (Wspólny dla 1P oraz tur w 2P)
    else if (currentState == PLAYER_TURN || currentState == PLAYER1_TURN || currentState == PLAYER2_TURN) {
        M5.Display.drawRect(OFFSET_X + aimCol * CELL_SIZE - 1, OFFSET_Y - 1, CELL_SIZE + 2, (GRID_SIZE * CELL_SIZE) + 2, 0x7BEF);
        M5.Display.drawRect(OFFSET_X + aimCol * CELL_SIZE + 1, OFFSET_Y + aimRow * CELL_SIZE + 1, CELL_SIZE - 2, CELL_SIZE - 2, WHITE);
        M5.Display.drawRect(OFFSET_X + aimCol * CELL_SIZE + 2, OFFSET_Y + aimRow * CELL_SIZE + 2, CELL_SIZE - 4, CELL_SIZE - 4, YELLOW);
    }
    // Celownik bota (Tylko w trybie vs AI)
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

void drawVictoryScreen(const char* msg) {
    M5.Display.fillScreen(BLACK);
    
    // Zwycięstwo - bezpieczny rozmiar 2 (nadal wyraźny, ale nie rozciąga się)
    M5.Display.setTextColor(GREEN);
    M5.Display.setTextSize(2);
    M5.Display.setTextDatum(CC_DATUM);
    M5.Display.drawString("ZWYCIESTWO!", 120, 25);
    
    // Kto kogo pokonał - mały, czytelny tekst
    M5.Display.setTextColor(WHITE);
    M5.Display.setTextSize(1);
    
    char buforTekstu[64];
    if (g_trybGraczy == 2) {
        // Obcinamy ewentualne spacje i składamy ładny komunikat
        snprintf(buforTekstu, sizeof(buforTekstu), "%s pokonal %s!", winnerName.c_str(), loserName.c_str());
        M5.Display.drawString(buforTekstu, 120, 65);
    } else {
        snprintf(buforTekstu, sizeof(buforTekstu), "Gratulacje %s!", imieGracza1.c_str());
        M5.Display.drawString(buforTekstu, 120, 65);
    }
    
    // Instrukcja powrotu na dół ekranu
    M5.Display.setTextColor(0x7BEF); // Nasz GRAY
    M5.Display.setTextSize(1);
    M5.Display.drawString("[ SPACJA = zagraj znowu ]", 120, 105);
}

void drawDefeatScreen(const char* msg) {
    M5.Display.fillScreen(BLACK);
    
    // Porażka
    M5.Display.setTextColor(RED);
    M5.Display.setTextSize(2);
    M5.Display.setTextDatum(CC_DATUM);
    M5.Display.drawString("PORAZKA!", 120, 25);
    
    // Komunikat o zatonięciu
    M5.Display.setTextColor(WHITE);
    M5.Display.setTextSize(1);
    
    char buforTekstu[64];
    snprintf(buforTekstu, sizeof(buforTekstu), "%s - flota zatonela...", imieGracza1.c_str());
    M5.Display.drawString(buforTekstu, 120, 65);
    
    // Instrukcja powrotu
    M5.Display.setTextColor(0x7BEF); // Nasz GRAY
    M5.Display.setTextSize(1);
    M5.Display.drawString("[ SPACJA = sprobuj znowu ]", 120, 105);
}