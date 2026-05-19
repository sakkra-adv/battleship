#include <M5Unified.h>
#include <M5Cardputer.h>
#include "Config.h"
#include "Logic.h"
#include "Display.h"

// Zmienne do obsługi blokady czasowej na ekranie końcowym
unsigned long gameOverStartTime = 0;
bool isTimerStarted = false;

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    M5Cardputer.begin(cfg, true);
    M5.Display.setRotation(1);
    
    Serial.begin(115200); 
    initLogic(); 
    drawUI();    
}

void loop() {
    M5Cardputer.update();

    // --- 1. BLOKADA I OBSŁUGA EKRANU KOŃCA GRY (VICTORY / DEFEAT) ---
    if (currentState == STATE_VICTORY || currentState == STATE_DEFEAT) {
        // Jeśli to pierwszy obieg pętli po zakończeniu gry, uruchamiamy stoper
        if (!isTimerStarted) {
            gameOverStartTime = millis();
            isTimerStarted = true;
            Serial.println("Uruchomiono blokade 4 sekund na ekranie koncowym...");
        }

        // Sprawdzamy, czy minęły już 4 sekundy (4000 ms)
        if (millis() - gameOverStartTime >= 4000) {
            // Dopiero po 4 sekundach pozwalamy na restart spacją
            if (M5Cardputer.Keyboard.isPressed() && M5Cardputer.Keyboard.isKeyPressed(' ')) {
                Serial.println("Restartowanie gry... Powrot do ustawiania statkow.");
                
                isTimerStarted = false; // Resetujemy flagę stopera
                initLogic();            // Reset logiki i plansz
                drawUI();               // Rysujemy od nowa ekran ustawiania
                
                delay(300); // Zapobiega natychmiastowemu kliknięciu w nowej grze
            }
        }
        
        return; // !!! KLUCZOWE: Przerywamy pętlę, blokujemy całą resztę sterowania gry
    }
    
    // --- 2. GŁÓWNA OBSŁUGA KLAWIATURY (TRWAJĄCA ROZGRYWKA) ---
    if (M5Cardputer.Keyboard.isPressed()) {
        bool inputAction = false;

        // --- FAZA: USTAWIANIE STATKÓW ---
        if (currentState == PLACING_SHIPS) {
            int nextCol = selectedCol;
            int nextRow = selectedRow;
            int nextRot = rotation;

            if (M5Cardputer.Keyboard.isKeyPressed(';') || M5Cardputer.Keyboard.isKeyPressed('e') || M5Cardputer.Keyboard.isKeyPressed('E'))      { nextRow--; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('.') || M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S')) { nextRow++; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed(',') || M5Cardputer.Keyboard.isKeyPressed('a') || M5Cardputer.Keyboard.isKeyPressed('A')) { nextCol--; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('/') || M5Cardputer.Keyboard.isKeyPressed('d') || M5Cardputer.Keyboard.isKeyPressed('D')) { nextCol++; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('r') || M5Cardputer.Keyboard.isKeyPressed('R')) { nextRot = (rotation + 1) % 4; inputAction = true; }

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

            if (M5Cardputer.Keyboard.isKeyPressed('c') || M5Cardputer.Keyboard.isKeyPressed('C')) {
                if (isLegalPlacement(selectedCol, selectedRow, currentShipIndex, rotation, true)) {
                    placeShip(selectedCol, selectedRow, currentShipIndex, rotation, true); 
                    currentShipIndex++;
                    selectedCol = 3; selectedRow = 3; rotation = 0;
                    
                    if (currentShipIndex >= SHIP_COUNT) {
                        placeComputerShips(); 
                        currentState = PLAYER_TURN; 
                    }
                    drawUI();
                    delay(300);
                }
            }
        }
        
        // --- FAZA: TURA GRACZA ---
        else if (currentState == PLAYER_TURN) {
            if (M5Cardputer.Keyboard.isKeyPressed('e') || M5Cardputer.Keyboard.isKeyPressed('E'))      { if(aimRow > 0) aimRow--; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S')) { if(aimRow < 7) aimRow++; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('a') || M5Cardputer.Keyboard.isKeyPressed('A')) { if(aimCol > 0) aimCol--; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('d') || M5Cardputer.Keyboard.isKeyPressed('D')) { if(aimCol < 7) aimCol++; inputAction = true; }

            else if (M5Cardputer.Keyboard.isKeyPressed('b') || M5Cardputer.Keyboard.isKeyPressed('B')) { aimCol = 1; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('c') || M5Cardputer.Keyboard.isKeyPressed('C')) { aimCol = 2; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('f') || M5Cardputer.Keyboard.isKeyPressed('F')) { aimCol = 5; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('g') || M5Cardputer.Keyboard.isKeyPressed('G')) { aimCol = 6; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('h') || M5Cardputer.Keyboard.isKeyPressed('H')) { aimCol = 7; inputAction = true; }

            if (M5Cardputer.Keyboard.isKeyPressed('1'))      { aimRow = 0; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('2')) { aimRow = 1; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('3')) { aimRow = 2; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('4')) { aimRow = 3; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('5')) { aimRow = 4; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('6')) { aimRow = 5; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('7')) { aimRow = 6; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('8')) { aimRow = 7; inputAction = true; }

            if (inputAction) {
                drawUI();
                delay(130);
                return;
            }
            
            // ODPALENIE TORPEDY (SPACE)
            if (M5Cardputer.Keyboard.isKeyPressed(' ')) {
                byte cell = enemyBoard[aimRow][aimCol];
                
                if (cell == 0 || (cell >= 10 && cell <= 13)) {
                    registerPlayerShot(aimCol, aimRow);
                    drawUI(); 
                    delay(1000); 

                    // Jeśli po naszym strzale gra się nie skończyła, pozwól rządzic EvilAI
                    if (currentState != STATE_VICTORY && currentState != STATE_DEFEAT) {
                        currentState = COMPUTER_TURN;
                        computerShot(); 
                        
                        // Jeśli bot nas nie wykończył, wracamy do tury gracza
                        if (currentState != STATE_VICTORY && currentState != STATE_DEFEAT) {
                            currentState = PLAYER_TURN; 
                            drawUI(); 
                        }
                    }
                } else {
                    Serial.println("Tam juz strzelales!");
                }
            }
        }
    }
}