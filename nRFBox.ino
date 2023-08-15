/* ____________________________
   This software is licensed under the MIT License:
   https://github.com/cifertech/nRFBox
   ________________________________________ */

#include <Arduino.h>
#include "menu.h"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_NeoPixel pixels(1, 5, NEO_GRB + NEO_KHZ800);


void setup() {

  pixels.begin();
 
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  menuSetup();
  
}

void loop() {

  menuLoop();

}
