#pragma once

#ifndef __UI_H__
#define __UI_H__

#include <TFT_eSPI.h>
#include <SPI.h>
#include <lvgl.h>
#include "printer.h"

#define COLOR_OFF lv_color_hex(0xa0a0a0)
#define COLOR_ON lv_color_hex(0x0ff00)
#define COLOR_BLACK lv_color_hex(0x000000)
#define COLOR_LABEL_PROGRESS lv_color_hex(0xffffff)
#define COLOR_LABEL_MESSAGE lv_color_hex(0xffffff)
#define COLOR_SPINNER_FAN_BACK lv_color_hex(0x303030)
#define COLOR_SPINNER_FAN lv_color_hex(0xa0a0a0)
#define COLOR_SPINNER_BACK lv_color_hex(0x303030)
#define COLOR_SPINNER lv_color_hex(0x00ff00)
#define COLOR_SPINNER_KNOB lv_color_hex(0x00ff00)

extern lv_obj_t *screen1;
extern lv_obj_t *screen2;
extern lv_obj_t *screen3;
extern lv_obj_t *label2;
extern lv_obj_t *updatearc;
extern lv_obj_t *preload;
extern unsigned long lastUpdate;

void lv_display_Init();
void lv_display_led_On();
void lv_display_led_Off();
void init_ui();
void update_screen_values(printer_values pValues);
#endif