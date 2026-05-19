#ifndef CONFIG_H
#define CONFIG_H

#include <M5Unified.h>

// --- GEOMETRIA I WYMIARY INTERFEJSU ---
const int GRID_SIZE = 8;
const int CELL_SIZE = 13; 
const int OFFSET_X = 115; 
const int OFFSET_Y = 12;

// --- OGÓLNA PALETA KOLORÓW ---
const uint16_t COLOR_BG      = BLACK;
const uint16_t COLOR_GRID    = BLUE;
const uint16_t COLOR_SHIP    = GREEN;
const uint16_t COLOR_TEXT    = WHITE;
const uint16_t COLOR_PLAYER  = GREEN;
const uint16_t COLOR_CURSOR  = WHITE;
const uint16_t COLOR_TARGET  = WHITE; // Biały element celownika / ogólne oznaczenie

#define GRAY 0x7BEF

// --- KONFIGURACJA UNIKALNEJ FLOTY ---
const int SHIP_COUNT = 4;
const int SHIP_SIZES[SHIP_COUNT] = {6, 4, 3, 2};

// Tablica kolorów idealnie zmapowana z indeksami obiektów w ALL_SHIPS:
// Index 0: Zygzak (6-masztowiec) -> CYAN
// Index 1: Litera T (4-masztowiec) -> MAGENTA
// Index 2: Klasyczny 3-masztowiec -> WHITE
// Index 3: Klasyczny 2-masztowiec -> GREEN
const uint16_t SHIP_COLORS[SHIP_COUNT] = {CYAN, MAGENTA, WHITE, GREEN};

// --- STANY GRY ---
enum GameState {
    START_MENU,
    PLACING_SHIPS,     // Tryb 1P (vs AI)
    PLAYER_TURN,       // Tryb 1P (vs AI)
    COMPUTER_TURN,     // Tryb 1P (vs AI)
    
    // --- NOWE STANY DLA TRYBU 2 GRACZY ---
    WAIT_P1_PLACE,     // Ekran: "Gracz 1, przygotuj sie"
    PLACING_P1,        // Rozstawianie okrętów przez Gracza 1
    WAIT_P2_PLACE,     // Ekran: "Gracz 2, przygotuj sie"
    PLACING_P2,        // Rozstawianie okrętów przez Gracza 2
    PLAYER1_TURN,      // Bitwa: Celowanie Gracza 1
    PLAYER2_TURN,      // Bitwa: Celowanie Gracza 2
    
    STATE_VICTORY,
    STATE_DEFEAT
};

#endif