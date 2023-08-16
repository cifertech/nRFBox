/* ____________________________
   This software is licensed under the MIT License:
   https://github.com/cifertech/nRFBox
   ________________________________________ */

#include <Arduino.h> 
#include "menu.h"

extern Adafruit_SSD1306 display;
extern Adafruit_NeoPixel pixels;

int selected =  0;
int entered  = -1;


void menuSetup() {

    pinMode(2, INPUT_PULLUP);
    pinMode(3, INPUT_PULLUP);
    pinMode(4, INPUT_PULLUP);
    pinMode(6, INPUT_PULLUP);

    display.clearDisplay();
  
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20, 5);
    display.println("nRF-BOX");
    display.setTextSize(1);
    display.setCursor(20, 20);
    display.println("by CiferTech");
    
    display.display();
    delay(3000);

    if (entered == -1)  scannerSetup();      
    if (entered == 1)   jammerSetup();
    if (entered == 2)   AnalyzerSetup();
}


void displaymenu(void) {

  int down =  digitalRead(2);
  int up =    digitalRead(3);
  int enter = digitalRead(4);
  int back  = digitalRead(6);
  

    if (up == LOW){
    selected = selected + 1;
    if (selected > 2)
    selected = 2;
    delay(200);
  };
    if (down == LOW){
    selected = selected - 1;
    if (selected < 0)
    selected = 0;
    delay(200);
  };
    if (enter == LOW) {
    entered = selected;
  };
    if (back == LOW) {
    entered = -1;
  };
    if (up == LOW && down == LOW){
    entered = -1;
  };

 
  const char *options[4] = {
    " Rf Scanner ",
    " 2.4 jammer ",
    " Analyzer "
  };


    switch (entered) {
    case -1: // Main Menu
      pixels.setPixelColor(0, pixels.Color(3, 0, 3));
      pixels.show();
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.println("nRF-BOX");

      for (int i = 0; i < 4; i++) {
        if (i == selected) {
          //display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
          display.print(">");
          display.println(options[i]);
        } else if (i != selected) {
          display.setTextColor(SSD1306_WHITE);
          display.println(options[i]);
        }
      }
      break;
      
    case 0:
      if (up == LOW) entered = -1;
      scannerLoop();
      break;

    case 1: 
      jammerLoop();
      break;

    case 2: 
      AnalyzerLoop();
      break;

    default:
      break;
  }

  display.display();
}

void menuLoop() {

  displaymenu();
  
}
