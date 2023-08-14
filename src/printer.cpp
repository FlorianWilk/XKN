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
#include "printer.h"

void get_printer_progress(printer_values *pValues)
{
  HTTPClient http;
  http.begin("http://" + klipper_ip + "/printer/objects/query?fan&display_status&temperature_sensor%20Chamber&extruder&heater_bed&print_stats&webhooks");
  int httpCode = http.GET();
  if (httpCode == 200)
  {
    String payload = http.getString();
    DynamicJsonDocument doc(payload.length() * 2);
    deserializeJson(doc, payload);
    pValues->progress = (doc["result"]["status"]["display_status"]["progress"].as<double>()) * 100.0;
    pValues->message = (doc["result"]["status"]["display_status"]["message"].as<String>());
    pValues->fan_speed = (uint16_t)((doc["result"]["status"]["fan"]["speed"].as<double>()) * 100.0);
    pValues->chamber_temp = (uint16_t)((doc["result"]["status"]["temperature_sensor Chamber"]["temperature"].as<double>()));
  }
  else
  {
    Serial.println("Error while getting status");
  }
  http.end();
}


void get_printer_status(printer_values *pValues)
{
  HTTPClient http;
  http.begin("http://" + klipper_ip + "/api/printer");
  int httpCode = http.GET();
  if (httpCode == 200)
  {
    String payload = http.getString();
    DynamicJsonDocument doc(payload.length() * 2);
    deserializeJson(doc, payload);
    String nameStr1 = doc["temperature"]["bed"]["actual"].as<String>();
    String nameStr2 = doc["temperature"]["bed"]["target"].as<String>();
    String nameStr3 = doc["temperature"]["tool0"]["actual"].as<String>();
    String nameStr4 = doc["temperature"]["tool0"]["target"].as<String>();
    String nameStr5 = doc["state"]["flags"]["printing"].as<String>();
    String nameStr6 = doc["state"]["flags"]["paused"].as<String>();

    bool isPrinting = doc["state"]["flags"]["printing"].as<bool>();
    pValues->is_printing = isPrinting;

    pValues->bedtemp_actual = (uint16_t)((doc["temperature"]["bed"]["actual"].as<double>()));
    pValues->bedtemp_target = (uint16_t)((doc["temperature"]["bed"]["target"].as<double>()));
    pValues->tooltemp_actual = (uint16_t)((doc["temperature"]["tool0"]["actual"].as<double>()));
    pValues->tooltemp_target = (uint16_t)((doc["temperature"]["tool0"]["target"].as<double>()));
  }
  else
  {
    Serial.println("Error while getting status");
  }
  http.end();
}