/* ____________________________
   This software is licensed under the MIT License:
   https://github.com/cifertech/nrfbox
   ________________________________________ */

#ifndef analyzer_H
#define analyzer_H

#include <SPI.h>
#include <U8g2lib.h>
#include "esp_bt.h"
#include "esp_wifi.h"

void analyzerSetup();
void analyzerLoop();

#endif
