#include "M5Wrapper.h"
#include <M5Cardputer.h>
#include "Display.h"
#include "Config.h"
#include "Logic.h"


// Deklaracje zewnętrzne
extern const lv_image_dsc_t IMG_0371;
extern lv_obj_t * ui_splash_screen; // Musimy widzieć ten obiekt, by wiedzieć czy intro trwa

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    M5Cardputer.begin(cfg, true);
    M5.Display.setRotation(1);
    
    Serial.begin(115200); 
    
    // 1. Inicjalizacja biblioteki LVGL (bez tego obrazek nie zadziała)
    // UWAGA: Zakładam, że masz lvgl_init() i sterowniki skonfigurowane w innym miejscu.
    // Jeśli nie, upewnij się, że Twoja konfiguracja LVGL jest poprawna.
    
    initLogic(); 
    
    // 2. Startujemy od obrazka, a nie od mapy gry
    create_splash_screen(); 
}

void loop() {
    M5Cardputer.update();
    
    // 3. Obsługa mrugania napisu i grafiki LVGL
    lv_timer_handler(); 

    // 4. SPRAWDZAMY: Czy trwa intro?
    if (ui_splash_screen != NULL) {
        // Jeśli intro trwa, wywołujemy funkcję z Logic.cpp, która czeka na klawisz '1' lub '2'
        handle_intro_input(); 
    } 
    // 5. JEŚLI NIE MA INTRA: Działa normalna logika gry
    else if (currentState == PLACING_SHIPS) {
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
                if (isWithinBounds(nextCol, nextRow, currentShipIndex, nextRot)) {
                    selectedCol = nextCol;
                    selectedRow = nextRow;
                    rotation = nextRot;
                    drawUI();
                }
                delay(130);
            }

            if (M5Cardputer.Keyboard.isKeyPressed('c')) {
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
}