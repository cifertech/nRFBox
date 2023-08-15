/* ____________________________
   This software is licensed under the MIT License:
   https://github.com/cifertech/nRFBox
   ________________________________________ */

#include <Arduino.h> 
#include "Analyzer.h"

extern Adafruit_SSD1306 display;
extern Adafruit_NeoPixel pixels;

RF24 radio(9, 10);

#define N 128
uint8_t value[N];

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

int xOffset = 0; 
int yOffset = 0;


void AnalyzerSetup(){

    Serial.begin(9600);

    display.clearDisplay();
    display.display();

    radio.begin();
    radio.setAutoAck(false);

}


void AnalyzerLoop(){

    memset(value, 0, sizeof(value));

    int n = 50;
    while (n--) {
      pixels.setPixelColor(0, pixels.Color(2, 0, 2));
      pixels.show();
        int i = N;
        while (i--) {
            radio.setChannel(i);
            radio.startListening();
            delayMicroseconds(128);
            radio.stopListening();
            if(radio.testCarrier()) {
                ++value[i];

              pixels.setPixelColor(0, pixels.Color(0, 2, 2));
              pixels.show();

            };
        }
    }

    display.clearDisplay();
    int barWidth = SCREEN_WIDTH / N;
    int x = xOffset;
    for (int i = 0; i < N; ++i) {
        int v = 31 - value[i] * 3 + yOffset; // Adjusted the y value
        if(v < 0) {
            v = 0;
        }
        display.drawFastVLine(x, v - 10, 32 - v, WHITE);
        x += barWidth;
    }
    display.setTextSize(1);      
    display.setTextColor(WHITE);  
    display.setCursor(0, SCREEN_HEIGHT - 8); 
    display.println("1..10...50...80...128"); // Print the range values
    display.display();

    delay(50);
}
