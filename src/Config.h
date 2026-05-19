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

// --- NOWA, WYSOKOKONTRASTOWA PALETA DLA DALTONISTÓW (RGB565) ---
const uint16_t SHIP_COLORS[SHIP_COUNT] = {
    0xFFFF, // 1. Statek: Czysty BIAŁY (Maksymalna jasność)
    0xFFE0, // 2. Statek: Jaskrawy ŻÓŁTY
    0xF81F, // 3. Statek: Głęboka MAGENTA / Różowy (Ciemniejszy, bardzo nasycony)
    0x3A55  // 4. Statek: Ciemny GRAFIT / Szary (Bardzo ciemny, kontrastowy do reszty)
};

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