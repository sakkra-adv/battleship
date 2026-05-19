#include <M5Unified.h>
#include <M5Cardputer.h>
#include "Config.h"
#include "Logic.h"
#include "Display.h"
#include "SplashImage.h"

unsigned long gameOverStartTime = 0;
bool isTimerStarted = false;
unsigned long ostatniBlink = 0;
bool kursorWidoczny = true;

// Deklaracja funkcji dźwiękowej (żeby kompilator jej szukał w Display.cpp)
void playSound(int freq, int durationMs, int type = 0);

// Funkcja pomocnicza do rysowania eleganckich ekranów informacyjnych
void pokazEkranPrzejsciowy(String tytul, String podpowiedz) {
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextColor(GOLD);
    M5.Display.setTextSize(1.5);
    M5.Display.setTextDatum(CC_DATUM);
    M5.Display.drawString(tytul.c_str(), 120, 45);
    M5.Display.setTextColor(WHITE);
    M5.Display.setTextSize(1);
    M5.Display.drawString(podpowiedz.c_str(), 120, 85);
}

// Funkcja pobierająca tekst z klawiatury (Przetłumaczona na język angielski)
String wpiszImie(String komunikat) {
    String wynik = "";
    bool enterPressed = false;
    delay(200);
    while (M5Cardputer.Keyboard.isPressed()) { M5Cardputer.update(); delay(10); }

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
                    playSound(600, 80); // Potwierdzenie imienia
                }
            }
            else if (M5Cardputer.Keyboard.isKeyPressed(KEY_BACKSPACE)) {
                if (wynik.length() > 0) { 
                    wynik.remove(wynik.length() - 1); 
                    trzebaOdswiezycPole = true; 
                    playSound(250, 40); // Dźwięk usuwania litery
                }
            }
            else if (wynik.length() < 6) { // Blokada do 6 znaków pod tabele
                char wpisanyZnak = 0;
                for (char c = 'a'; c <= 'z'; c++) if (M5Cardputer.Keyboard.isKeyPressed(c)) { wpisanyZnak = c; break; }
                for (char c = 'A'; c <= 'Z'; c++) if (M5Cardputer.Keyboard.isKeyPressed(c)) { wpisanyZnak = c; break; }
                for (char c = '0'; c <= '9'; c++) if (M5Cardputer.Keyboard.isKeyPressed(c)) { wpisanyZnak = c; break; }
                if (M5Cardputer.Keyboard.isKeyPressed(' ')) wpisanyZnak = ' ';

                if (wpisanyZnak != 0) {
                    if (wynik.length() == 0) wynik += (char)toupper(wpisanyZnak);
                    else wynik += (char)tolower(wpisanyZnak);
                    trzebaOdswiezycPole = true;
                    playSound(450, 40); // Dźwięk wpisywania litery
                    delay(40); 
                }
            }
            delay(150); 
        }
        if (trzebaOdswiezycPole) {
            M5.Display.fillRect(11, 51, 218, 28, BLACK);
            M5.Display.setTextColor(GREEN);
            M5.Display.setTextSize(2);
            M5.Display.setCursor(20, 56);
            M5.Display.print(wynik);
            if (kursorWidoczny) M5.Display.print("_");
            trzebaOdswiezycPole = false;
        }
        delay(10);
    }
    return wynik;
}

void pokazSplash() {
    currentState = START_MENU;
    M5.Display.setSwapBytes(true); 
    M5.Display.pushImage(0, 0, 240, 135, splash_bitmap);
}

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    M5Cardputer.begin(cfg, true);
    
    // --- AKTYWACJA GŁOŚNIKA ---
    M5.Speaker.begin();
    M5.Speaker.setVolume(64); // Skala 0-255 (64 to optymalne, ciche retro)
    
    M5.Display.setRotation(1);
    Serial.begin(115200); 
    initLogic(); 
    pokazSplash();
}

void loop() {
    M5Cardputer.update();

    // --- FAZA: MENU GŁÓWNE (Teksty po angielsku) ---
    if (currentState == START_MENU) {
        if (M5Cardputer.Keyboard.isPressed()) {
            if (M5Cardputer.Keyboard.isKeyPressed('1')) {
                g_trybGraczy = 1;
                playSound(500, 100);
                imieGracza1 = wpiszImie("Enter Captain's name:");
                imieGracza2 = "EvilAI"; 
                initLogic(); 
                placeComputerShips();
                currentState = PLACING_SHIPS; 
                drawUI();
            }
            else if (M5Cardputer.Keyboard.isKeyPressed('2')) {
                g_trybGraczy = 2;
                playSound(500, 100);
                imieGracza1 = wpiszImie("Player 1 - Enter name:");
                imieGracza2 = " " + wpiszImie("Player 2 - Enter name:");
                reset2PlayerBoards();
                
                currentState = WAIT_P1_PLACE;
                pokazEkranPrzejsciowy("Captain " + imieGracza1, "Deploy your fleet! [Press SPACE]");
            }
        }
        return;
    }

    // --- FAZA: EKRANY OCZEKIWANIA NA ROZSTAWIENIE (2 GRACZY) ---
    if (currentState == WAIT_P1_PLACE || currentState == WAIT_P2_PLACE) {
        if (M5Cardputer.Keyboard.isPressed() && M5Cardputer.Keyboard.isKeyPressed(' ')) {
            
            // Śluza: Czekamy na całkowite puszczenie spacji
            delay(200);
            while (M5Cardputer.Keyboard.isPressed()) { M5Cardputer.update(); delay(10); }

            playSound(587, 150); // Akceptacja spacji przed rozstawianiem

            // Czyszczenie ręczne bez psucia stanów:
            selectedCol = 3; selectedRow = 3; rotation = 0;
            currentShipIndex = 0; 
            for(int r=0; r<8; r++) {
                for(int c=0; c<8; c++) {
                    playerBoard[r][c] = 0;
                    enemyBoard[r][c] = 0;
                }
            }
            
            currentState = (currentState == WAIT_P1_PLACE) ? PLACING_P1 : PLACING_P2;
            drawUI();
        }
        return;
    }

    // --- FAZA: BLOKADA KOŃCA GRY ---
    if (currentState == STATE_VICTORY || currentState == STATE_DEFEAT) {
        if (!isTimerStarted) { gameOverStartTime = millis(); isTimerStarted = true; }
        if (millis() - gameOverStartTime >= 4000) {
            if (M5Cardputer.Keyboard.isPressed() && M5Cardputer.Keyboard.isKeyPressed(' ')) {
                playSound(440, 100);
                isTimerStarted = false; 
                pokazSplash(); 
                delay(300); 
            }
        }
        return; 
    }
    
    // --- GŁÓWNA PĘTLA OBSŁUGI STEROWANIA (GRA I ROZSTAWIENIE) ---
    if (M5Cardputer.Keyboard.isPressed()) {
        bool inputAction = false;

        // --- OBSŁUGA ROZSTAWIANIA STATKÓW (WSPÓLNA DLA 1P I 2P) ---
        if (currentState == PLACING_SHIPS || currentState == PLACING_P1 || currentState == PLACING_P2) {
            int nextCol = selectedCol; int nextRow = selectedRow; int nextRot = rotation;

            if (M5Cardputer.Keyboard.isKeyPressed(';') || M5Cardputer.Keyboard.isKeyPressed('e') || M5Cardputer.Keyboard.isKeyPressed('E'))      { nextRow--; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('.') || M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S')) { nextRow++; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed(',') || M5Cardputer.Keyboard.isKeyPressed('a') || M5Cardputer.Keyboard.isKeyPressed('A')) { nextCol--; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('/') || M5Cardputer.Keyboard.isKeyPressed('d') || M5Cardputer.Keyboard.isKeyPressed('D')) { nextCol++; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('r') || M5Cardputer.Keyboard.isKeyPressed('R')) { nextRot = (rotation + 1) % 4; inputAction = true; }

            if (inputAction) {
                if (isWithinBounds(nextCol, nextRow, currentShipIndex, nextRot)) {
                    selectedCol = nextCol; selectedRow = nextRow; rotation = nextRot; drawUI();
                    playSound(400, 30); // Krótkie piknięcie przy ruchu/obracaniu statku
                }
                delay(130); return;
            }

            // NOWOŚĆ: Zatwierdzenie pozycji statku za pomocą SPACJI zamiast klawisza 'C'
            if (M5Cardputer.Keyboard.isKeyPressed(' ')) {
                if (isLegalPlacement(selectedCol, selectedRow, currentShipIndex, rotation, true)) {
                    placeShip(selectedCol, selectedRow, currentShipIndex, rotation, true); 
                    currentShipIndex++;
                    selectedCol = 3; selectedRow = 3; rotation = 0;
                    
                    playSound(700, 80); // Dźwięk pomyślnego postawienia statku
                    
                    // Bezpieczeństwo: Czekamy, aż gracz fizycznie puści spację, żeby zapobiec podwójnemu kliknięciu
                    delay(200);
                    while (M5Cardputer.Keyboard.isPressed()) { M5Cardputer.update(); delay(10); }
                    
                    if (currentShipIndex >= SHIP_COUNT) {
                        if (currentState == PLACING_SHIPS) { // Tryb 1P kończy bieg
                            placeComputerShips(); currentState = PLAYER_TURN; 
                        } 
                        else if (currentState == PLACING_P1) { // Gracz 1 skończył -> czas na Gracza 2
                            savePlayer1View();

                            currentState = WAIT_P2_PLACE;
                            // Tekst zmieniony na angielski
                            pokazEkranPrzejsciowy("Captain " + imieGracza2, "Deploy your fleet! [Press SPACE]");
                            return;
                        }
                        else if (currentState == PLACING_P2) { // Gracz 2 skończył -> odpalamy bitwę!
                            memcpy(p2Fleet, playerBoard, sizeof(p2Fleet)); // Zapis floty G2

                            currentState = PLAYER1_TURN;
                            loadPlayer1View(); // Zaczyna zawsze Gracz 1
                            aimCol = 3; aimRow = 3;
                        }
                    }
                    drawUI();
                    delay(100);
                } else {
                    playSound(180, 150); // ERROR SOUND: złe rozstawienie statku
                    delay(150);
                }
            }
        }
        
        // --- OBSŁUGA STRZELANIA (WSPÓLNA DLA TURY GRACZA 1P ORAZ OBU GRACZY W 2P) ---
        else if (currentState == PLAYER_TURN || currentState == PLAYER1_TURN || currentState == PLAYER2_TURN) {
            if (M5Cardputer.Keyboard.isKeyPressed('e') || M5Cardputer.Keyboard.isKeyPressed('E'))      { if(aimRow > 0) aimRow--; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S')) { if(aimRow < 7) aimRow++; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('a') || M5Cardputer.Keyboard.isKeyPressed('A')) { if(aimCol > 0) aimCol--; inputAction = true; }
            else if (M5Cardputer.Keyboard.isKeyPressed('d') || M5Cardputer.Keyboard.isKeyPressed('D')) { if(aimCol < 7) aimCol++; inputAction = true; }

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
                playSound(350, 30); // Krótkie piknięcie celownika
                delay(130); 
                return; 
            }
            
            if (M5Cardputer.Keyboard.isKeyPressed(' ')) {
                byte cell = enemyBoard[aimRow][aimCol];
                if (cell == 0 || (cell >= 10 && cell <= 13)) {
                    GameState turaPrzedStrzalem = currentState; // Zapamiętujemy, kto strzelał
                    
                    // Odpalenie dźwięku wystrzału torpedy
                    playSound(900, 50, 1);
                    playSound(500, 80, 1);

                    bool hit = registerPlayerShot(aimCol, aimRow);
                    
                    if (hit) {
                        // DWUFAZOWY WYBUCH: Trafienie!
                        playSound(800, 80, 1);
                        playSound(300, 150, 1);
                    } else {
                        // Niskie retro "plum" w wodę: Pudło!
                        playSound(150, 200);
                    }

                    drawUI(); 
                    delay(1000); 

                    if (currentState == STATE_VICTORY) { // Ktoś wygrał!
                        if (g_trybGraczy == 2) {
                            winnerName = (turaPrzedStrzalem == PLAYER1_TURN) ? imieGracza1 : imieGracza2;
                            loserName = (turaPrzedStrzalem == PLAYER1_TURN) ? imieGracza2 : imieGracza1;
                        }
                        drawUI(); 
                    } else {
                        if (g_trybGraczy == 1) { // Tryb vs AI leci klasycznie
                            currentState = COMPUTER_TURN;
                            computerShot(); 
                            if (currentState != STATE_VICTORY && currentState != STATE_DEFEAT) {
                                currentState = PLAYER_TURN; drawUI(); 
                            }
                        } else { // Tryb 2P: Rotujemy plansze i zmieniamy gracza
                            if (turaPrzedStrzalem == PLAYER1_TURN) {
                                savePlayer1View(); loadPlayer2View(); currentState = PLAYER2_TURN;
                            } else {
                                savePlayer2View(); loadPlayer1View(); currentState = PLAYER1_TURN;
                            }
                            aimCol = 3; aimRow = 3; // Reset pozycji celownika dla nowego gracza
                            drawUI();
                        }
                    }
                }
            }
        }
    }
}