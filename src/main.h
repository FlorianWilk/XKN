#ifndef __MAIN_H__
#define __MAIN_H__

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
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "printer.h"
#include "ui.h"
#include "myconfig.h"

bool isUpdating = false;

Ticker timer1;
String text_print_status = "standby";        
String text_print_file_name = "No Printfile"; 


uint32_t last_http_request = 0;
int laststatus = 0;

printer_values pValues;


#endif