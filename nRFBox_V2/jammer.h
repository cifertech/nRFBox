/* ____________________________
   This software is licensed under the MIT License:
   https://github.com/cifertech/nrfbox
   ________________________________________ */
   
#ifndef jammer_H
#define jammer_H

#include <SPI.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <RF24.h>
#include "esp_bt.h"
#include "esp_wifi.h"

void jammerSetup();
void jammerLoop();

#endif
