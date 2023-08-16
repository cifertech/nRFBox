/* ____________________________
   This software is licensed under the MIT License:
   https://github.com/cifertech/nRFBox
   ________________________________________ */

#ifndef menu_H
#define menu_H

#include "scanner.h"
#include "jammer.h"
#include "Analyzer.h"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

void menuSetup();
void menuLoop();

#endif
