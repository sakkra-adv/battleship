#include <M5Unified.h>
#include <M5Cardputer.h>
#include "Config.h"
#include "Logic.h"
#include "Display.h"

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    M5Cardputer.begin(cfg, true);
    M5.Display.setRotation(1);
    
    // Inicjalizacja portu szeregowego do debugowania
    Serial.begin(115200); 
    
    initLogic(); 
    drawUI();    
}

void loop() {
    M5Cardputer.update();
    
    if (M5Cardputer.Keyboard.isPressed()) {
        bool inputAction = false;

        // --- FAZA 1: USTAWIANIE STATKÓW ---
        if (currentState == PLACING_SHIPS) {
            int nextCol = selectedCol;
            int nextRow = selectedRow;
            int nextRot = rotation;

            if (M5Cardputer.Keyboard.isKeyPressed(';'))      { nextRow--; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('.')) { nextRow++; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed(',')) { nextCol--; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('/')) { nextCol++; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('r')) { nextRot = (rotation + 1) % 4; inputAction = true; }

            if (inputAction) {
                if (isWithinBounds(nextCol, nextRow, currentShipIndex, nextRot)) {
                    selectedCol = nextCol;
                    selectedRow = nextRow;
                    rotation = nextRot;
                    drawUI();
                }
                delay(130);
                return;
            }

            if (M5Cardputer.Keyboard.isKeyPressed('c')) {
                if (canPlaceShip(selectedCol, selectedRow, currentShipIndex, rotation)) {
                    placeShip(selectedCol, selectedRow, currentShipIndex, rotation);
                    currentShipIndex++;
                    selectedCol = 3; selectedRow = 3; rotation = 0;
                    if (currentShipIndex >= SHIP_COUNT) {
                        currentState = PLAYER_TURN; // Przełączamy na bitwę!
                    }
                    drawUI();
                    delay(300);
                }
            }
        }
        
        // --- FAZA 2: TURA GRACZA (CELOWANIE LITERAMI I CYFRAMI + SPACJA) ---
        else if (currentState == PLAYER_TURN) {
            
            // 1. Wybór kolumny (A - H) - małe i duże litery
            if (M5Cardputer.Keyboard.isKeyPressed('a') || M5Cardputer.Keyboard.isKeyPressed('A')) { aimCol = 0; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('b') || M5Cardputer.Keyboard.isKeyPressed('B')) { aimCol = 1; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('c') || M5Cardputer.Keyboard.isKeyPressed('C')) { aimCol = 2; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('d') || M5Cardputer.Keyboard.isKeyPressed('D')) { aimCol = 3; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('e') || M5Cardputer.Keyboard.isKeyPressed('E')) { aimCol = 4; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('f') || M5Cardputer.Keyboard.isKeyPressed('F')) { aimCol = 5; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('g') || M5Cardputer.Keyboard.isKeyPressed('G')) { aimCol = 6; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('h') || M5Cardputer.Keyboard.isKeyPressed('H')) { aimCol = 7; inputAction = true; }

            // 2. Wybór rzędu (1 - 8)
            if (M5Cardputer.Keyboard.isKeyPressed('1'))      { aimRow = 0; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('2')) { aimRow = 1; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('3')) { aimRow = 2; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('4')) { aimRow = 3; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('5')) { aimRow = 4; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('6')) { aimRow = 5; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('7')) { aimRow = 6; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('8')) { aimRow = 7; inputAction = true; }

            // Jeśli zmieniono współrzędne, odświeżamy ekran z nową pozycją celownika
            if (inputAction) {
                drawUI();
                delay(130);
                return;
            }

            // 3. SPUST - SPACJA 
            if (M5Cardputer.Keyboard.isKeyPressed(' ')) {
                // Blokada strzelania dwa razy w to samo miejsce
                if (enemyBoard[aimRow][aimCol] == 0) {
                    Serial.printf("POTWIERDZONO OGIEŃ! Strzal w pozycje: %c%d\n", 'A' + aimCol, aimRow + 1);
                    
                    // Test wizualny: Na razie oznaczamy strzał jako trafiony (3), żeby zobaczyć czerwony kafelek
                    enemyBoard[aimRow][aimCol] = 3; 
                    
                    drawUI();
                    delay(400); // krótka pauza na efekt uderzenia
                } else {
                    Serial.println("Leszek, tam już leciała rakieta! Wybierz inne współrzędne.");
                }
            }
        }
    }
}