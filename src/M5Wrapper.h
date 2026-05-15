#ifndef M5_WRAPPER_H
#define M5_WRAPPER_H

// 1. Najpierw Twoje główne LVGL
#include <lvgl.h>

// 2. Blokujemy plik z M5GFX zanim zostanie załadowany przez M5Unified
#define LGFX_LVGL_H_
#define LV_FONT_FMT_TXT_H

// 3. Teraz M5
#include <M5Unified.h>
#include <M5Cardputer.h>

#endif