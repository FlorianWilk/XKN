#include <Arduino.h>
#include "User_Setup.h"
#include <TFT_eSPI.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <Ticker.h>
#include <lvgl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <test.h>
#include "printer.h"
#include "ui.h"

TFT_eSPI tft = TFT_eSPI(240, 240);
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[TFT_WIDTH * 10];

lv_style_t screenstyle;
lv_style_t spinnerstyle;
lv_style_t spinnerstyle2;
lv_style_t spinnerstyle3;
lv_style_t spinnerstyle4;
lv_style_t style_label_logo;
lv_style_t style_label_status;
lv_style_t style_label_progress;
lv_style_t style_label_current;
lv_style_t style_label_current_cold;
lv_style_t style_label_target;
lv_style_t style_label_message;
lv_style_t knob_style;
lv_style_t style_label_fan;

lv_obj_t *screen1;
lv_obj_t *screen2;
lv_obj_t *screen3;

lv_obj_t *label2;
lv_obj_t *preload;
lv_obj_t *updatearc;

lv_obj_t *tool_cur_temp_label;
lv_obj_t *bed_cur_temp_label;
lv_obj_t *tool_target_temp_label;
lv_obj_t *bed_target_temp_label;
lv_obj_t *progress_label;
lv_obj_t *progress_arc;
lv_obj_t *label_fan_speed;
lv_obj_t *label_chamber_temp;
lv_obj_t *arc_fan;
lv_obj_t *label_message;

String lastMessage = "";
bool screensaver_active = false;
unsigned long lastUpdate;

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();                                        // 使能写功能
  tft.setAddrWindow(area->x1, area->y1, w, h);             // 设置填充区域
  tft.pushColors((uint16_t *)&color_p->full, w * h, true); // 写入颜色缓存和缓存大小
  tft.endWrite();                                          // 关闭写功能

  lv_disp_flush_ready(disp); // 调用区域填充颜色函数
}

void lv_display_Init()
{
  tft.init();
  tft.setRotation(0);
  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, TFT_WIDTH * 10);

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  /*Change the following line to your display resolution*/
  disp_drv.hor_res = TFT_WIDTH;
  disp_drv.ver_res = TFT_HEIGHT;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);
}

void lv_display_led_On()
{
  pinMode(16, OUTPUT);    // 旧版本
  digitalWrite(16, HIGH); // 背光默认开始

  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH); // 背光默认开始
}

void lv_display_led_Off()
{
  pinMode(16, OUTPUT);   // 旧版本
  digitalWrite(16, LOW); // 背光默认开始

  pinMode(2, OUTPUT);
  digitalWrite(2, LOW); // 背光默认开始
}



void init_styles(){
 // Styles

  lv_style_set_text_font(&style_label_current_cold, &lv_font_montserrat_22);
  lv_style_set_text_color(&style_label_current_cold, lv_color_hex(0xa0a0a0));

  lv_style_set_text_font(&style_label_logo, &lv_font_montserrat_40);
  lv_style_set_text_color(&style_label_logo, lv_color_hex(0x00FF000));

  lv_style_set_text_font(&style_label_progress, &lv_font_montserrat_40);
  lv_style_set_text_color(&style_label_progress, COLOR_LABEL_PROGRESS);

  lv_style_set_text_font(&style_label_message, &lv_font_montserrat_22);
  lv_style_set_text_color(&style_label_message, COLOR_LABEL_MESSAGE);

  lv_style_set_text_font(&style_label_current, &lv_font_montserrat_22);
  lv_style_set_text_color(&style_label_current, lv_color_hex(0x00ff00));

  lv_style_set_text_font(&style_label_target, &lv_font_montserrat_18);
  lv_style_set_text_color(&style_label_target, lv_color_hex(0xa0a0a0));

  lv_style_set_arc_color(&spinnerstyle4, COLOR_SPINNER_FAN_BACK);
  lv_style_set_arc_color(&spinnerstyle3, COLOR_SPINNER_FAN);
  lv_style_set_arc_width(&spinnerstyle4, 4);
  lv_style_set_arc_width(&spinnerstyle3, 4);

  lv_style_set_arc_color(&spinnerstyle, COLOR_SPINNER_BACK);
  lv_style_set_arc_color(&spinnerstyle2, COLOR_SPINNER);

  lv_style_set_bg_color(&knob_style, COLOR_SPINNER_KNOB);
  lv_style_set_pad_all(&knob_style, -1);

  lv_style_set_bg_color(&screenstyle, COLOR_BLACK);
}

void init_ui()
{
  lv_display_Init();
  lv_display_led_On();
  Serial.println("Booting");

  lv_disp_set_bg_color(NULL, lv_color_hex(0x000000));

  init_styles();

  // Welcome Screen

  screen1 = lv_obj_create(NULL);

  lv_obj_add_style(screen1, &screenstyle, LV_PART_MAIN);

  lv_obj_t *label1 = lv_label_create(screen1);
  lv_obj_add_style(label1, &style_label_logo, LV_PART_MAIN);
  lv_label_set_text(label1, "XKN");
  lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

  label2 = lv_label_create(screen1);
  lv_style_set_text_font(&style_label_status, &lv_font_montserrat_22);
  lv_style_set_text_color(&style_label_status, lv_color_hex(0xa0a0a0));
  lv_obj_add_style(label2, &style_label_status, LV_PART_MAIN);
  lv_label_set_text(label2, "Booting");
  lv_obj_align(label2, LV_ALIGN_CENTER, 0, 28);

  preload = lv_spinner_create(screen1, 5000, 60);
  lv_obj_set_size(preload, 239, 239);
  lv_obj_align(preload, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_style(preload, &spinnerstyle, LV_PART_MAIN);
  lv_obj_add_style(preload, &spinnerstyle2, LV_PART_INDICATOR);
  lv_scr_load(screen1);

  updatearc = lv_arc_create(screen1);
  lv_obj_set_size(updatearc, 239, 239);
  lv_obj_align(updatearc, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_style(updatearc, &spinnerstyle, LV_PART_MAIN);
  lv_obj_add_style(updatearc, &spinnerstyle2, LV_PART_INDICATOR);
  lv_obj_add_style(updatearc, &knob_style, LV_PART_KNOB);
  lv_arc_set_rotation(updatearc, 270);
  lv_arc_set_bg_angles(updatearc, 0, 360);
  lv_arc_set_value(updatearc, 0);
  lv_obj_clear_flag(updatearc, LV_OBJ_FLAG_CLICKABLE);
  lv_scr_load(screen1);
  lv_obj_add_flag(updatearc, LV_OBJ_FLAG_HIDDEN);

  // Printing Screen
  screen2 = lv_obj_create(NULL);
  // lv_style_set_bg_color(&screenstyle, lv_color_hex(0x000000));
  lv_obj_add_style(screen2, &screenstyle, LV_PART_MAIN);

  progress_label = lv_label_create(screen2);
  lv_obj_add_style(progress_label, &style_label_progress, LV_PART_MAIN);
  lv_label_set_text(progress_label, "-");
  lv_obj_align(progress_label, LV_ALIGN_CENTER, 0, 0);

  label_message = lv_label_create(screen2);
  lv_obj_add_style(label_message, &style_label_message, LV_PART_MAIN);
  lv_label_set_text(label_message, "-");
  lv_obj_align(label_message, LV_ALIGN_CENTER, 0, 0);

  bed_cur_temp_label = lv_label_create(screen2);
  lv_obj_add_style(bed_cur_temp_label, &style_label_current, LV_PART_MAIN);
  lv_label_set_text(bed_cur_temp_label, "-");
  lv_obj_align(bed_cur_temp_label, LV_ALIGN_CENTER, 35, 45);

  bed_target_temp_label = lv_label_create(screen2);
  lv_obj_add_style(bed_target_temp_label, &style_label_target, LV_PART_MAIN);
  lv_label_set_text(bed_target_temp_label, "-");
  lv_obj_align(bed_target_temp_label, LV_ALIGN_CENTER, 35, 65);

  tool_cur_temp_label = lv_label_create(screen2);
  lv_obj_add_style(tool_cur_temp_label, &style_label_current, LV_PART_MAIN);
  lv_label_set_text(tool_cur_temp_label, "-");
  lv_obj_align(tool_cur_temp_label, LV_ALIGN_CENTER, -35, 45);

  tool_target_temp_label = lv_label_create(screen2);
  lv_obj_add_style(tool_target_temp_label, &style_label_target, LV_PART_MAIN);
  lv_label_set_text(tool_target_temp_label, "-");
  lv_obj_align(tool_target_temp_label, LV_ALIGN_CENTER, -35, 65);

  // lv_style_copy(&style_label_fan,&style_label_current);
  label_fan_speed = lv_label_create(screen2);
  lv_obj_add_style(label_fan_speed, &style_label_current, LV_PART_MAIN);
  lv_label_set_text(label_fan_speed, "-");
  lv_obj_align(label_fan_speed, LV_ALIGN_CENTER, 35, -50);

  arc_fan = lv_arc_create(screen2);
  lv_obj_set_size(arc_fan, 30, 30);
  // lv_obj_align(arc_fan, LV_ALIGN_CENTER, 62, -38);
  lv_obj_align_to(arc_fan, label_fan_speed, LV_ALIGN_OUT_RIGHT_MID, -12, 13);
  //  lv_obj_align_to(arc_fan, label_fan_speed, LV_ALIGN_RIGHT_MID, 23, 13);
  lv_obj_add_style(arc_fan, &spinnerstyle4, LV_PART_MAIN);
  lv_obj_add_style(arc_fan, &spinnerstyle3, LV_PART_INDICATOR);
  lv_arc_set_rotation(arc_fan, 270);
  lv_arc_set_bg_angles(arc_fan, 0, 270);
  lv_arc_set_value(arc_fan, 0);
  lv_obj_remove_style(arc_fan, NULL, LV_PART_KNOB);
  lv_obj_clear_flag(arc_fan, LV_OBJ_FLAG_CLICKABLE);

  // ---

  label_chamber_temp = lv_label_create(screen2);
  lv_obj_add_style(label_chamber_temp, &style_label_current, LV_PART_MAIN);
  lv_label_set_text(label_chamber_temp, "-");
  lv_obj_align(label_chamber_temp, LV_ALIGN_CENTER, -35, -50);

  progress_arc = lv_arc_create(screen2);
  lv_obj_set_size(progress_arc, 239, 239);
  lv_obj_align(progress_arc, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_style(progress_arc, &spinnerstyle, LV_PART_MAIN);
  lv_obj_add_style(progress_arc, &spinnerstyle2, LV_PART_INDICATOR);
  lv_obj_add_style(progress_arc, &knob_style, LV_PART_KNOB);
  lv_arc_set_rotation(progress_arc, 270);
  lv_arc_set_bg_angles(progress_arc, 0, 360);
  lv_arc_set_value(progress_arc, 0);
  lv_obj_remove_style(progress_arc, NULL, LV_PART_KNOB);
  lv_obj_clear_flag(progress_arc, LV_OBJ_FLAG_CLICKABLE);

  // Screensaver
  screen3 = lv_obj_create(NULL);
}



void update_screen_values(printer_values pValues)
{
  String text_ext_actual_temp = String(pValues.tooltemp_actual, 10) + "°C";
  String text_ext_target_temp = String(pValues.tooltemp_target, 10) + "°C";
  String text_bed_actual_temp = String(pValues.bedtemp_actual, 10) + "°C";
  String text_bed_target_temp = String(pValues.bedtemp_target, 10) + "°C";

  lv_style_value_t v_off = {.color = COLOR_OFF};
  lv_style_value_t v_on = {.color = COLOR_ON};

  // Bed Temp

  if (pValues.bedtemp_actual < 50 && pValues.bedtemp_target == 0)
  {
    lv_obj_set_local_style_prop(bed_cur_temp_label, LV_STYLE_TEXT_COLOR, v_off, LV_PART_MAIN);
  }
  else
  {
    lv_obj_set_local_style_prop(bed_cur_temp_label, LV_STYLE_TEXT_COLOR, v_on, LV_PART_MAIN);
  }

  lv_label_set_text(bed_cur_temp_label, text_bed_actual_temp.c_str());
  lv_label_set_text(bed_target_temp_label, text_bed_target_temp.c_str());

  if (pValues.bedtemp_target == 0)
  {
    lv_obj_add_flag(bed_target_temp_label, LV_OBJ_FLAG_HIDDEN);
  }
  else
  {
    lv_obj_clear_flag(bed_target_temp_label, LV_OBJ_FLAG_HIDDEN);
  }

  // Tool Temp

  if (pValues.tooltemp_actual < 50 && pValues.tooltemp_target == 0)
  {
    lv_obj_set_local_style_prop(tool_cur_temp_label, LV_STYLE_TEXT_COLOR, v_off, LV_PART_MAIN);
  }
  else
  {
    lv_obj_set_local_style_prop(tool_cur_temp_label, LV_STYLE_TEXT_COLOR, v_on, LV_PART_MAIN);
  }

  lv_label_set_text(tool_cur_temp_label, text_ext_actual_temp.c_str());
  lv_label_set_text(tool_target_temp_label, text_ext_target_temp.c_str());

  if (pValues.tooltemp_target == 0)
  {
    lv_obj_add_flag(tool_target_temp_label, LV_OBJ_FLAG_HIDDEN);
  }
  else
  {
    lv_obj_clear_flag(tool_target_temp_label, LV_OBJ_FLAG_HIDDEN);
  }

  // Chamber Temp

  if (pValues.chamber_temp < 30 && pValues.bedtemp_target == 0 && pValues.tooltemp_target == 0 && pValues.bedtemp_actual < 50)
  {
    lv_obj_set_local_style_prop(label_chamber_temp, LV_STYLE_TEXT_COLOR, v_off, LV_PART_MAIN);
  }
  else
  {
    lv_obj_set_local_style_prop(label_chamber_temp, LV_STYLE_TEXT_COLOR, v_on, LV_PART_MAIN);
  }

  lv_label_set_text(label_chamber_temp, (String(pValues.chamber_temp, 10) + "°C").c_str());

  // Message

  if (pValues.message != "null")
  {
    lv_label_set_text(label_message, pValues.message.c_str());
  }
  else
  {
    lv_label_set_text(label_message, "Welcome");
  }
  int16_t progress_data = 0;

  // Progress

  String nameStrpriting = "0";
  progress_data = pValues.progress;
  uint16_t datas = (uint16_t)(progress_data);

  if (datas == 0)
  {
    nameStrpriting = "0%";
  }
  else
  {
    nameStrpriting = String(datas, 10) + "%";
  }
  lv_arc_set_value(progress_arc, progress_data);

  lv_label_set_text(progress_label, nameStrpriting.c_str());

  // FAN Speed

  if (pValues.fan_speed == 0)
  {
    lv_obj_set_local_style_prop(label_fan_speed, LV_STYLE_TEXT_COLOR, v_off, LV_PART_MAIN);
  }
  else
  {
    lv_obj_set_local_style_prop(label_fan_speed, LV_STYLE_TEXT_COLOR, v_on, LV_PART_MAIN);
  }
  lv_label_set_text(label_fan_speed, (String(pValues.fan_speed, 10) + "%").c_str());
  lv_arc_set_value(arc_fan, pValues.fan_speed);
  lv_obj_align_to(arc_fan, label_fan_speed, LV_ALIGN_OUT_RIGHT_MID, -12, 13);

  // Screentype

  if (pValues.is_printing && pValues.bedtemp_target > 0 && pValues.tooltemp_target > 0 && pValues.progress > 1)
  {
    lv_obj_clear_flag(progress_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(progress_arc, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(label_message, LV_OBJ_FLAG_HIDDEN);
  }
  else
  {
    lv_obj_add_flag(progress_label, LV_OBJ_FLAG_HIDDEN);
    lv_arc_set_value(progress_arc, 0);
    lv_obj_clear_flag(label_message, LV_OBJ_FLAG_HIDDEN);
  }

  // Screensaver logic

  if (lastMessage != pValues.message)
  {
    lastUpdate = millis();
    lastMessage = pValues.message;
  }

  if (pValues.is_printing || pValues.bedtemp_actual > 50 || pValues.bedtemp_target > 0 || pValues.tooltemp_target > 0 || pValues.tooltemp_actual > 50)
  {
    lastUpdate = millis();
  }

  // Screensaver activate

  if (millis() - lastUpdate > 1000 * 60 * 10)
  {
    if (!screensaver_active)
    {
      screensaver_active = true;
      lv_display_led_Off();
      lv_scr_load(screen3);
    }
  }
  else
  {
    if (screensaver_active)
    {
      lv_scr_load(screen2);
      screensaver_active = false;
      lv_display_led_On();
    }
  }
}