#ifndef LV_CONF_H
#define LV_CONF_H

/* Ustawienia ekranu Cardputera */
#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0

#include <stdint.h>

/* Włączamy konfigurację */
#define LV_CONF_SKIP 0
#define LV_USE_USER_DATA 1



/* Pamięć - ustawiamy standardowo */
#define LV_MEM_CUSTOM 0
#define LV_MEM_SIZE (32U * 1024U)
#define LV_MEM_ADR 0

/* Odświeżanie */
#define LV_DISP_DEF_REFR_PERIOD 30

/* Włączamy potrzebne moduły */
#define LV_USE_IMG 1
#define LV_USE_LABEL 1
#define LV_USE_ANIMATION 1

/* Czcionka */
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_DEFAULT &lv_font_montserrat_14

#endif