/* ____________________________
   This software is licensed under the MIT License:
   https://github.com/cifertech/nRFBox
   ________________________________________ */

#ifndef scanner_H
#define scanner_H

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

extern Adafruit_SSD1306 display;

void scannerSetup();
void scannerLoop();

#endif
