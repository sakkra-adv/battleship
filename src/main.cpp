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
        int nextCol = selectedCol;
        int nextRow = selectedRow;
        int nextRot = rotation;
        bool inputAction = false;

        if (M5Cardputer.Keyboard.isKeyPressed(';'))      { nextRow--; inputAction = true; }
        else if (M5Cardputer.Keyboard.isKeyPressed('.')) { nextRow++; inputAction = true; }
        else if (M5Cardputer.Keyboard.isKeyPressed(',')) { nextCol--; inputAction = true; }
        else if (M5Cardputer.Keyboard.isKeyPressed('/')) { nextCol++; inputAction = true; }
        else if (M5Cardputer.Keyboard.isKeyPressed('r')) { nextRot = (rotation + 1) % 4; inputAction = true; }

        if (inputAction) {
            // Sprawdzamy TYLKO, czy klocki nie wychodzą poza krawędź ekranu 8x8
            // Nie blokujemy już ruchu przez inne statki!
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
            // Tutaj canPlaceShip nadal pilnuje zasad (brak kolizji)
            if (canPlaceShip(selectedCol, selectedRow, currentShipIndex, rotation)) {
                placeShip(selectedCol, selectedRow, currentShipIndex, rotation);
                currentShipIndex++;
                selectedCol = 3; selectedRow = 3; rotation = 0;
                if (currentShipIndex >= SHIP_COUNT) currentState = PLAYER_TURN;
                drawUI();
                delay(300);
            }
        }
    }
}