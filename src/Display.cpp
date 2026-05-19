#include "Display.h"
#include "Config.h"
#include "Logic.h"

void drawUI() {
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
    }

    M5.Display.setTextColor(COLOR_PLAYER);
    M5.Display.setCursor(5, 17); 
    M5.Display.print("Player: Leszek");

    // --- 2. PANEL GRACZA / LEWA LEGENDA ---
    M5.Display.drawRect(5, 35, 85, 95, COLOR_PLAYER);
    M5.Display.drawLine(60, 35, 60, 130, COLOR_PLAYER); 
    M5.Display.setTextColor(COLOR_TEXT);
    M5.Display.setCursor(10, 38);
    M5.Display.print("hits:");

    for(int i = 0; i < SHIP_COUNT; i++) {
        int yPos = 50 + (i * 20);
        for(int b = 0; b < SHIP_SIZES[i]; b++) {
            M5.Display.fillRect(10 + b*7, yPos, 5, 8, SHIP_COLORS[i]);
        }
        M5.Display.setCursor(70, yPos);
        M5.Display.printf("0"); 
    }

    // --- 3. MAPA I LOGIKA WYŚWIETLANIA (PRAWA STRONA) ---
    for (int r = 0; r < GRID_SIZE; r++) {
        // Cyfry po lewej (1-8)
        M5.Display.setTextColor(COLOR_TEXT);
        M5.Display.setCursor(OFFSET_X - 12, OFFSET_Y + r * CELL_SIZE + 3);
        M5.Display.printf("%d", r + 1);

        for (int c = 0; c < GRID_SIZE; c++) {
            uint16_t cellColor = COLOR_GRID; // Domyślny kolor obwódki (niebieski)
            bool fillCell = false;
            uint16_t fillColor = COLOR_GRID;

            // Faza 1: Ustawianie statków - Widzimy planszę gracza (playerBoard)
            if (currentState == PLACING_SHIPS) {
                byte cellValue = playerBoard[r][c];
                
                // Jeśli na polu znajduje się jakikolwiek postawiony statek (ID: 10, 11, 12, 13)
                if (cellValue >= 10 && cellValue <= 13) {
                    int shipIdx = cellValue - 10;
                    cellColor = SHIP_COLORS[shipIdx];
                    fillColor = SHIP_COLORS[shipIdx];
                    fillCell = true;
                }
                
                if (fillCell) {
                    M5.Display.fillRect(OFFSET_X + c * CELL_SIZE + 2, OFFSET_Y + r * CELL_SIZE + 2, CELL_SIZE - 4, CELL_SIZE - 4, fillColor);
                }
                M5.Display.drawRect(OFFSET_X + c * CELL_SIZE, OFFSET_Y + r * CELL_SIZE, CELL_SIZE, CELL_SIZE, cellColor);
            }
            
            // Faza 2: Bitwa - Widzimy planszę bota (enemyBoard) z uwzględnieniem Mgły Wojny
            else if (currentState == PLAYER_TURN) {
                byte cellValue = enemyBoard[r][c];

                if (cellValue == 1) { 
                    // Pudło gracza (np. ciemnoniebieski / szary kwadracik lub kropka)
                    cellColor = BLUE;
                    fillColor = BLUE;
                    fillCell = true;
                } 
                else if (cellValue >= 30 && cellValue <= 33) { 
                    // Celne trafienie! Dekodujemy unikalny indeks statku (odjmujemy maskę 30)
                    int shipIdx = cellValue - 30;
                    fillColor = SHIP_COLORS[shipIdx]; // Pobieramy oryginalny kolor (Cyan, Magenta itd.)
                    cellColor = fillColor;
                    fillCell = true;
                }
                // UWAGA: Jeśli cellValue wynosi od 10 do 13 (żywy statek bota) lub 0 (czysta woda) 
                // – fillCell pozostaje 'false', co oznacza kompletną, szczelną mgłę wojny.

                if (fillCell) {
                    M5.Display.fillRect(OFFSET_X + c * CELL_SIZE + 2, OFFSET_Y + r * CELL_SIZE + 2, CELL_SIZE - 4, CELL_SIZE - 4, fillColor);
                    M5.Display.drawRect(OFFSET_X + c * CELL_SIZE, OFFSET_Y + r * CELL_SIZE, CELL_SIZE, CELL_SIZE, cellColor);
                    
                    // Opcjonalny wizualny akcent dla trafienia: mały czarny krzyżyk 'X' wewnątrz kafelka
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

    // --- 4. ELEMENTY SPECJALNE (DUCH STATKU / CELOWNIK) ---
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
        // Podświetlenie pionowej kolumny celownika
        M5.Display.drawRect(OFFSET_X + aimCol * CELL_SIZE - 1, 
                            OFFSET_Y - 1, 
                            CELL_SIZE + 2, (GRID_SIZE * CELL_SIZE) + 2, 0x7BEF);

        // Celownik dynamiczny biało-żółty
        M5.Display.drawRect(OFFSET_X + aimCol * CELL_SIZE + 1, 
                            OFFSET_Y + aimRow * CELL_SIZE + 1, 
                            CELL_SIZE - 2, CELL_SIZE - 2, WHITE);
        M5.Display.drawRect(OFFSET_X + aimCol * CELL_SIZE + 2, 
                            OFFSET_Y + aimRow * CELL_SIZE + 2, 
                            CELL_SIZE - 4, CELL_SIZE - 4, YELLOW);
    }

    // Litery A-H pod mapą
    M5.Display.setTextColor(COLOR_TEXT);
    for (int i = 0; i < GRID_SIZE; i++) {
        M5.Display.setCursor(OFFSET_X + i * CELL_SIZE + 4, OFFSET_Y + (GRID_SIZE * CELL_SIZE) + 4);
        M5.Display.printf("%c", 'A' + i);
    }

    M5.Display.endWrite();
}