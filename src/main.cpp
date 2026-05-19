#include <M5Unified.h>
#include <M5Cardputer.h>
#include "Config.h"
#include "Logic.h"
#include "Display.h"
#include "SplashImage.h" // Upewnij się, że masz ten plik w projekcie!

unsigned long gameOverStartTime = 0;
bool isTimerStarted = false;

unsigned long ostatniBlink = 0;
bool kursorWidoczny = true;

// Płynna i stabilna funkcja wpisywania imienia z Cardputera
String wpiszImie(String komunikat) {
    String wynik = "";
    bool enterPressed = false;
    
    delay(200);
    while (M5Cardputer.Keyboard.isPressed()) {
        M5Cardputer.update();
        delay(10);
    }

    M5.Display.fillScreen(BLACK);
    M5.Display.setTextColor(CYAN);
    M5.Display.setTextSize(1.5);
    M5.Display.setTextDatum(TL_DATUM); 
    M5.Display.setCursor(10, 20);
    M5.Display.println(komunikat);
    M5.Display.drawRect(10, 50, 220, 30, WHITE);

    bool trzebaOdswiezycPole = true;

    while (!enterPressed) {
        M5Cardputer.update();

        if (millis() - ostatniBlink > 300) {
            kursorWidoczny = !kursorWidoczny;
            ostatniBlink = millis();
            trzebaOdswiezycPole = true;
        }

        if (M5Cardputer.Keyboard.isPressed()) {
            if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) {
                if (wynik.length() > 0) { 
                    enterPressed = true;
                }
            }
            else if (M5Cardputer.Keyboard.isKeyPressed(KEY_BACKSPACE)) {
                if (wynik.length() > 0) {
                    wynik.remove(wynik.length() - 1);
                    trzebaOdswiezycPole = true;
                }
            }
            // --- POPRAWIONY OGRANICZNIK: Maksymalnie 6 znaków ---
            else if (wynik.length() < 6) {
                char wpisanyZnak = 0;

                // Szukamy, który klawisz został wciśnięty
                for (char c = 'a'; c <= 'z'; c++) {
                    if (M5Cardputer.Keyboard.isKeyPressed(c)) { wpisanyZnak = c; break; }
                }
                for (char c = 'A'; c <= 'Z'; c++) {
                    if (M5Cardputer.Keyboard.isKeyPressed(c)) { wpisanyZnak = c; break; }
                }
                for (char c = '0'; c <= '9'; c++) {
                    if (M5Cardputer.Keyboard.isKeyPressed(c)) { wpisanyZnak = c; break; }
                }
                if (M5Cardputer.Keyboard.isKeyPressed(' ')) {
                    wpisanyZnak = ' ';
                }

                // Jeśli wykryliśmy jakiś znak...
                if (wpisanyZnak != 0) {
                    // --- INTELIGENTNA DUŻA LITERA ---
                    if (wynik.length() == 0) {
                        // Pierwszy znak zamieniamy na dużą literę
                        wynik += (char)toupper(wpisanyZnak);
                    } else {
                        // Każdy kolejny znak zostaje małą literą (lub cyfrą/spacją)
                        wynik += (char)tolower(wpisanyZnak);
                    }

                    trzebaOdswiezycPole = true;
                    
                    // --- NOWOŚĆ: Malutki delay po wpisaniu litery ---
                    delay(40); 
                }
            }
            delay(150); // Standardowy debouncing, żeby litery nie wskakiwały podwójnie
        }

        if (trzebaOdswiezycPole) {
            M5.Display.fillRect(11, 51, 218, 28, BLACK);
            M5.Display.setTextColor(GREEN);
            M5.Display.setTextSize(2);
            M5.Display.setCursor(20, 56);
            M5.Display.print(wynik);
            
            if (kursorWidoczny) {
                M5.Display.print("_");
            }
            trzebaOdswiezycPole = false;
        }
        delay(10);
    }
    return wynik;
}

// Pomocnicza funkcja pokazująca Twój piękny ekran startowy
void pokazSplash() {
    currentState = START_MENU;
    M5.Display.setSwapBytes(true); // Gwarancja idealnych kolorów z Sposobu 2
    M5.Display.pushImage(0, 0, 240, 135, splash_bitmap);
    Serial.println("Ekran powitalny aktywny. Czekam na klawisz [1] lub [2]...");
}

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    M5Cardputer.begin(cfg, true);
    M5.Display.setRotation(1);
    
    Serial.begin(115200); 
    
    initLogic(); 
    pokazSplash(); // Startujemy od obrazka!
}

void loop() {
    M5Cardputer.update();

    // --- STAN: MENU STARTOWE (CZEKAMY NA 1 LUB 2) ---
    if (currentState == START_MENU) {
        if (M5Cardputer.Keyboard.isPressed()) {
            if (M5Cardputer.Keyboard.isKeyPressed('1')) {
                g_trybGraczy = 1;
                imieGracza1 = wpiszImie("Wpisz imie Kapitana:");
                imieGracza2 = "EvilAI"; // W trybie 1 gracza przeciwnikiem jest bot
                
                // Konfigurujemy plansze pod nową rozgrywkę
                initLogic(); 
                currentState = PLACING_SHIPS; // Przechodzimy do gry!
                drawUI(); // Rysujemy ramki bitwy
            }
            else if (M5Cardputer.Keyboard.isKeyPressed('2')) {
                // Kod dla dwóch graczy zrobimy w następnym kroku, na razie mrugnijmy ekranem
                M5.Display.fillScreen(BLUE);
                delay(200);
                pokazSplash();
            }
        }
        return; // Blokujemy resztę gry dopóki jesteśmy w menu
    }

    // --- STAN: BLOKADA EKRANU KOŃCOWEGO (VICTORY / DEFEAT) ---
    if (currentState == STATE_VICTORY || currentState == STATE_DEFEAT) {
        if (!isTimerStarted) {
            gameOverStartTime = millis();
            isTimerStarted = true;
        }

        if (millis() - gameOverStartTime >= 4000) {
            if (M5Cardputer.Keyboard.isPressed() && M5Cardputer.Keyboard.isKeyPressed(' ')) {
                isTimerStarted = false; 
                // Po restarcie wracamy na sam początek – do ekranu tytułowego!
                pokazSplash(); 
                delay(300); 
            }
        }
        return; 
    }
    
    // --- GŁÓWNA OBSŁUGA KLAWIATURY (TRWAJĄCA ROZGRYWKA) ---
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

                    if (currentState != STATE_VICTORY && currentState != STATE_DEFEAT) {
                        currentState = COMPUTER_TURN;
                        computerShot(); 
                        
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