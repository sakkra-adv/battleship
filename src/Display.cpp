#include "M5Wrapper.h"
#include "Display.h"
#include "Config.h"
#include "Logic.h"
#include <M5Cardputer.h>

// Deklaracja obrazka
extern const lv_image_dsc_t IMG_0371;

// Zmienne globalne dla LVGL
lv_obj_t * ui_splash_screen;
lv_obj_t * ui_start_label;

// Funkcja pomocnicza do animacji (mrugania)
void anim_opacity_cb(void * var, int32_t v) {
    lv_obj_set_style_opa((lv_obj_t *)var, v, 0);
}

void create_splash_screen() {
    // 1. Tworzymy ekran startowy
    ui_splash_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(ui_splash_screen, lv_color_hex(0x000000), 0);

    // 2. Obrazek tła (statki)
    lv_obj_t * img = lv_image_create(ui_splash_screen);
    lv_image_set_src(img, &IMG_0371);
    lv_obj_center(img);

    // 3. Mrugający napis
    ui_start_label = lv_label_create(ui_splash_screen);
    lv_label_set_text(ui_start_label, "PRESS 1 OR 2 PLAYERS");
    lv_obj_set_style_text_color(ui_start_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(ui_start_label, LV_ALIGN_BOTTOM_MID, 0, -15);

    // 4. Animacja mrugania (wypełniam to, co było puste)
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, ui_start_label);
    lv_anim_set_values(&a, 255, 0); // Od widocznego do zniknięcia
    lv_anim_set_time(&a, 800);
    lv_anim_set_playback_time(&a, 400);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_exec_cb(&a, anim_opacity_cb);
    lv_anim_start(&a);
    
    // Załaduj ekran
    lv_scr_load(ui_splash_screen);
}

// Funkcja do usuwania intro, gdy zaczynamy grę
void delete_splash_screen() {
    if(ui_splash_screen != NULL) {
        lv_obj_del(ui_splash_screen);
        ui_splash_screen = NULL;
    }
}

// Twoja stara funkcja drawUI (zostaje bez zmian, użyjesz jej jak gra ruszy)
void drawUI() {
    M5.Display.startWrite();
    // ... reszta Twojego kodu rysującego mapę ...
    M5.Display.endWrite();
}