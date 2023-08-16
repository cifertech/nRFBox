/* ____________________________
   This software is licensed under the MIT License:
   https://github.com/cifertech/nRFBox
   ________________________________________ */

#ifndef Analyzer_H
#define Analyzer_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

void AnalyzerSetup();
void AnalyzerLoop();

#endif
