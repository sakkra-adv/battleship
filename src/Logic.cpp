#include "Logic.h"
#include "Config.h"
#include "Display.h"  // Musimy to dodać, żeby móc wywołać drawUI() i delete_splash_screen()
#include <M5Cardputer.h>
#include "M5Wrapper.h"

// Deklarujemy, że ui_splash_screen istnieje gdzieś w projekcie (w Display.cpp)
extern lv_obj_t * ui_splash_screen;

GameState currentState = PLACING_SHIPS;
int selectedCol = 3;
int selectedRow = 3;
int rotation = 0;
int currentShipIndex = 0;
byte playerBoard[8][8] = {0};

const ShipShape ALL_SHIPS[4] = {
    {6, {{0,0}, {1,0}, {2,0}, {1,1}, {2,1}, {3,1}}}, 
    {4, {{0,0}, {-1,0}, {1,0}, {0,1}}},             
    {3, {{0,0}, {1,0}, {2,0}}},                     
    {2, {{0,0}, {1,0}}}                             
};

// --- NOWA FUNKCJA: OBSŁUGA EKRANU STARTOWEGO ---
void handle_intro_input() {
    // Sprawdzamy czy ekran startowy jest aktualnie wyświetlany
    if (ui_splash_screen != NULL) {
        // Jeśli jakikolwiek klawisz zostanie naciśnięty (lub konkretnie 1 lub 2)
        if (M5Cardputer.Keyboard.isKeyPressed('1') || M5Cardputer.Keyboard.isKeyPressed('2')) {
            
            // 1. Usuwamy obrazek statków
            delete_splash_screen(); 
            
            // 2. Czyścimy ekran przed rysowaniem mapy
            M5.Display.fillScreen(BLACK); 
            
            // 3. Ustawiamy stan na układanie statków
            currentState = PLACING_SHIPS; 
            
            // 4. Rysujemy pierwszy raz mapę gry
            drawUI(); 
            
            Serial.println("Gra wystartowała!");
        }
    }
}

void initLogic() {
    // Na początku ustawiamy stan na INTRO (możesz dodać taki stan do GameState w Config.h)
    // Jeśli nie chcesz dodawać nowego stanu, po prostu startujemy i czekamy na handle_intro_input
    selectedCol = 3;
    selectedRow = 3;
    rotation = 0;
    currentShipIndex = 0;
    for(int r=0; r<8; r++) for(int c=0; c<8; c++) playerBoard[r][c] = 0;
}

Point rotatePoint(Point p, int rot) {
    if (rot == 1) return {-p.dy, p.dx};
    if (rot == 2) return {-p.dx, -p.dy};
    if (rot == 3) return {p.dy, -p.dx};
    return p;
}

bool canPlaceShip(int col, int row, int shipIdx, int rot) {
    const ShipShape& s = ALL_SHIPS[shipIdx];
    for (int i = 0; i < s.size; i++) {
        Point p = rotatePoint(s.modules[i], rot);
        int c = col + p.dx;
        int r = row + p.dy;
        if (c < 0 || c >= GRID_SIZE || r < 0 || r >= GRID_SIZE) return false;
        if (playerBoard[r][c] != 0 && playerBoard[r][c] != 4) return false; 
    }
    return true;
}

bool isWithinBounds(int col, int row, int shipIdx, int rot) {
    const ShipShape& s = ALL_SHIPS[shipIdx];
    for (int i = 0; i < s.size; i++) {
        Point p = rotatePoint(s.modules[i], rot);
        int c = col + p.dx;
        int r = row + p.dy;
        if (c < 0 || c >= 8 || r < 0 || r >= 8) return false;
    }
    return true;
}

void placeShip(int col, int row, int shipIdx, int rot) {
    const ShipShape& s = ALL_SHIPS[shipIdx];
    for (int i = 0; i < s.size; i++) {
        Point p = rotatePoint(s.modules[i], rot);
        int c = col + p.dx;
        int r = row + p.dy;
        playerBoard[r][c] = 1;
        for (int dr = -1; dr <= 1; dr++) {
            for (int dc = -1; dc <= 1; dc++) {
                int nr = r + dr; int nc = c + dc;
                if (nr >= 0 && nr < GRID_SIZE && nc >= 0 && nc < GRID_SIZE) {
                    if (playerBoard[nr][nc] == 0) playerBoard[nr][nc] = 4;
                }
            }
        }
    }
}