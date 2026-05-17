#include "Display.h"
#include "Config.h"
#include "Logic.h"

void drawUI() {
    M5.Display.startWrite();
    M5.Display.fillScreen(COLOR_BG);
    
    // --- 1. PROSTY PASEK INFORMACYJNY (GÓRA EKRANU, OD LEWEJ) ---
    M5.Display.setTextSize(1);
    
    // Linia 1 (Górna, Y=3): Skrócone komunikaty, aby bez problemu mieściły się przed mapą
    M5.Display.setCursor(5, 3); 
    if (currentState == PLACING_SHIPS) {
        M5.Display.setTextColor(YELLOW);
        M5.Display.print("Setup: Arrows, R=Rotate, C=Confirm");
    } else if (currentState == PLAYER_TURN) {
        M5.Display.setTextColor(COLOR_TEXT);
        M5.Display.print("Aim (e.g. B4) + [SPACE] to fire");
    }

    // Linia 2 (Dolna, Y=15): Kto gra
    M5.Display.setTextColor(COLOR_PLAYER);
    M5.Display.setCursor(5, 17); 
    M5.Display.print("Player: Leszek");

        // --- 2. PANEL GRACZA (LEWA STRONA, OBNIŻONY O PARĘ PIKSELI DLA PORZĄDKU) ---
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
            uint16_t cellColor = COLOR_GRID;
            bool fillCell = false;
            uint16_t fillColor = COLOR_GRID;

            if (currentState == PLACING_SHIPS) {
                if (playerBoard[r][c] == 1) {
                    cellColor = COLOR_SHIP;
                    fillColor = COLOR_SHIP;
                    fillCell = true;
                }
            }
            else if (currentState == PLAYER_TURN) {
                if (enemyBoard[r][c] == 2) {
                    cellColor = BLUE;
                    fillColor = BLUE;
                    fillCell = true;
                } 
                else if (enemyBoard[r][c] == 3) {
                    cellColor = COLOR_TARGET;
                    fillColor = COLOR_TARGET;
                    fillCell = true;
                }
            }

            if (fillCell) {
                M5.Display.fillRect(OFFSET_X + c * CELL_SIZE + 2, OFFSET_Y + r * CELL_SIZE + 2, CELL_SIZE - 4, CELL_SIZE - 4, fillColor);
            }
            M5.Display.drawRect(OFFSET_X + c * CELL_SIZE, OFFSET_Y + r * CELL_SIZE, CELL_SIZE, CELL_SIZE, cellColor);
        }
    }

    // --- 4. ELEMENTY SPECJALNE (DUCH STATKU / CELOWNIK DLA CELL_SIZE = 13) ---
    if (currentState == PLACING_SHIPS) {
        bool legal = canPlaceShip(selectedCol, selectedRow, currentShipIndex, rotation);
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
        // Podświetlenie pionowej kolumny (dopasowane symetrycznie do nowej krawędzi)
        M5.Display.drawRect(OFFSET_X + aimCol * CELL_SIZE - 1, 
                            OFFSET_Y - 1, 
                            CELL_SIZE + 2, (GRID_SIZE * CELL_SIZE) + 2, 0x7BEF);

        // Celownik dynamiczny biało-żółty (dokładnie wyskalowany do wnętrza kratki 13x13)
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