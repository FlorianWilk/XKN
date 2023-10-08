#include <Arduino.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
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
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "main.h"
#include "ui.h"

unsigned long previous_wificheck_millis = 0;
unsigned long wificheck_interval = 30000;

void ui_timer()
{
  lv_tick_inc(1); 
}

// OTA-Things ------------------------------------------------------------------

void onOTA_begin()
{
  isUpdating = true;
  lv_display_led_On();
  lv_scr_load(screen1);
  lv_label_set_text(label2, "Updating");
  lv_obj_add_flag(preload, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(updatearc, LV_OBJ_FLAG_HIDDEN);
  lv_task_handler();
  lv_tick_inc(1);
  String type;
  if (ArduinoOTA.getCommand() == U_FLASH)
    type = "sketch";
  else // U_SPIFFS
    type = "filesystem";

  // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
  Serial.println("Start updating " + type);
}

void onOTA_end()
{
  Serial.println("\nEnd");
  lv_label_set_text(label2, "Finished");
  lv_obj_add_flag(updatearc, LV_OBJ_FLAG_HIDDEN);
  lv_task_handler();
  lv_tick_inc(1);
}

void onOTA_progress(unsigned int progress, unsigned int total){
Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
                    lv_arc_set_value(updatearc,(progress / (total / 100)));   
                       lv_task_handler();
      lv_tick_inc(1);
}

void initOTA()
{
  ArduinoOTA.onStart(&onOTA_begin).onEnd(&onOTA_end).onProgress(&onOTA_progress)
      .onError([](ota_error_t error)
               {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed"); });

  ArduinoOTA.begin();
}

// WIFI-Things ------------------------------------------------------------------

void connect_wifi(){
  lv_label_set_text(label2, "Connecting");
  lv_task_handler();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    lv_task_handler();
    delay(10);
  }
  lv_label_set_text(label2, "Connected");
  lv_task_handler();

  initOTA();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void check_wifi(){
  unsigned long currentMillis = millis();
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previous_wificheck_millis >= wificheck_interval))
  {
    ESP.restart();

    // This currently makes no sense bcs ESO is restarted. TODO: Make it nicer
    WiFi.disconnect();
    WiFi.reconnect();
    ArduinoOTA.begin();
    previous_wificheck_millis = currentMillis;
  }
}

// MAIN-Things ------------------------------------------------------------------

void setup()
{
  Serial.begin(115200);
  init_ui();
  lv_scr_load(screen1);

  // Background Timer
  timer1.attach(0.001, ui_timer);

  // Wait a second because of yeah
  for (int i = 0; i < 100; i++)
  {
    lv_task_handler();
    delay(10);
  }

  connect_wifi();

  lv_label_set_text(label2, "Ready");
  lv_task_handler();

  lv_scr_load(screen2);
  lastUpdate = millis();
}

void loop()
{
  ArduinoOTA.handle();
  lv_task_handler();

  check_wifi();

  if (!isUpdating)
  {
    uint32_t httprequest_nowtime = millis();
    if (httprequest_nowtime - last_http_request > 1000)
    {
      switch (laststatus)
      {
      case 0:
        get_printer_status(&pValues);
        break;
      case 1:
        get_printer_progress(&pValues);
      default:
        break;
      }
      laststatus++;
      laststatus %= 2;
      last_http_request = httprequest_nowtime;
      update_screen_values(pValues);
    }
  }
}
