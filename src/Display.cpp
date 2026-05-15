#include "Display.h"
#include "Config.h"
#include "Logic.h"

void drawUI() {
    M5.Display.startWrite();
    M5.Display.fillScreen(COLOR_BG);
    
    // --- 1. PANEL GRACZA (LEWA STRONA) ---
    M5.Display.setTextColor(COLOR_PLAYER);
    M5.Display.setTextSize(1);
    M5.Display.setCursor(5, 2);
    M5.Display.print("Player:");
    M5.Display.setCursor(10, 12);
    M5.Display.setTextSize(2);
    M5.Display.print("Leszek");

    M5.Display.drawRect(5, 35, 85, 95, COLOR_PLAYER);
    M5.Display.drawLine(60, 35, 60, 130, COLOR_PLAYER); 
    M5.Display.setTextSize(1);
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

    // --- 2. MAPA I LOGIKA WYŚWIETLANIA (PRAWA STRONA) ---
    for (int r = 0; r < GRID_SIZE; r++) {
        // Cyfry po lewej od mapy
        M5.Display.setTextColor(COLOR_TEXT);
        M5.Display.setCursor(OFFSET_X - 12, OFFSET_Y + r * CELL_SIZE + 3);
        M5.Display.printf("%d", r + 1);

        for (int c = 0; c < GRID_SIZE; c++) {
            uint16_t color = COLOR_GRID;

if (playerBoard[r][c] == 1) {
    color = COLOR_SHIP;
    // Wypełnij środek klocka, żeby nie był tylko ramką
    M5.Display.fillRect(OFFSET_X + c * CELL_SIZE + 2, OFFSET_Y + r * CELL_SIZE + 2, CELL_SIZE - 4, CELL_SIZE - 4, color);
}

if (currentState == PLACING_SHIPS) {
    // Sprawdzamy, czy w obecnej pozycji można postawić statek
    bool legal = canPlaceShip(selectedCol, selectedRow, currentShipIndex, rotation);
    uint16_t ghostColor = legal ? YELLOW : RED; // Żółty jak OK, Czerwony jak kolizja

    const ShipShape& s = ALL_SHIPS[currentShipIndex];
    for (int i = 0; i < s.size; i++) {
        Point p = rotatePoint(s.modules[i], rotation);
        int drawC = selectedCol + p.dx;
        int drawR = selectedRow + p.dy;
        
        // Rysujemy klocki ducha
        M5.Display.drawRect(OFFSET_X + drawC * CELL_SIZE + 1, 
                            OFFSET_Y + drawR * CELL_SIZE + 1, 
                            CELL_SIZE - 2, CELL_SIZE - 2, ghostColor);
    }
}
            
            // Rysowanie kwadracika siatki
            M5.Display.drawRect(OFFSET_X + c * CELL_SIZE, OFFSET_Y + r * CELL_SIZE, CELL_SIZE, CELL_SIZE, color);
            
            // Wypełnienie, jeśli to postawiony statek, żeby był lepiej widoczny
            if (playerBoard[r][c] == 1) {
                M5.Display.fillRect(OFFSET_X + c * CELL_SIZE + 2, OFFSET_Y + r * CELL_SIZE + 2, CELL_SIZE - 4, CELL_SIZE - 4, color);
            }
        }
    }

    // Litery A-H pod mapą
    M5.Display.setTextColor(COLOR_TEXT);
    for (int i = 0; i < GRID_SIZE; i++) {
        M5.Display.setCursor(OFFSET_X + i * CELL_SIZE + 4, OFFSET_Y + (GRID_SIZE * CELL_SIZE) + 4);
        M5.Display.printf("%c", 'A' + i);
    }

    M5.Display.endWrite();
}