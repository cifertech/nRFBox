/* ____________________________
   This software is licensed under the MIT License:
   https://github.com/cifertech/nRFBox
   ________________________________________ */

#include <Arduino.h> 
#include "jammer.h"

#define BT1 2
#define BT2 3

extern Adafruit_SSD1306 display;
extern Adafruit_NeoPixel pixels;

const int  num_channels = 64;
int        values[num_channels];
int        valuesDisplay[32];
int        channel = 0;
const int  num_reps = 50;
bool       jamming = false;
const byte address[6] = "00001";


void radioConfig() {
  digitalWrite(9, HIGH); 
  delayMicroseconds(150); 

  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  digitalWrite(10, LOW); 

  SPI.transfer(0x20 | (0x06 & 0x07));
  SPI.transfer(0x07);

  SPI.transfer(0x20 | (0x05 & 0x07));
  SPI.transfer(0x08);

  digitalWrite(10, HIGH); 
  SPI.endTransaction();
}


void radioWrite(const void* buf, uint8_t len) {
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  digitalWrite(10, LOW); 

  SPI.transfer(0xA0);
  for (uint8_t i = 0; i < len; i++) {
    SPI.transfer(((const uint8_t*)buf)[i]);
  }

  digitalWrite(10, HIGH); 
  SPI.endTransaction();

  delayMicroseconds(150);
}


void radioSetChannel(int channel) {
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  digitalWrite(10, LOW); 

  SPI.transfer(0x20 | (0x05 & 0x07));
  SPI.transfer(0x90 | (channel & 0x7F));

  digitalWrite(10, HIGH); 
  SPI.endTransaction();
}


void jammer() {
  const char text[] = "xxxxxxxxxxxxxxxx"; 
  for (int i = ((channel * 5) + 1); i < ((channel * 5) + 23); i++) {
    radioSetChannel(i);
    radioWrite(&text, sizeof(text));
  }
}


void pressBt01() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > 200) {
    if (channel < 13) {
      channel++;
    } else {
      channel = 0;
    }
  }
  last_interrupt_time = interrupt_time;
  pixels.setPixelColor(0, pixels.Color(5, 0, 5));
  pixels.show();
}


void pressBt02() {
  jamming = !jamming;   
  delay(200);
}


void jammerSetup() {
  
  Serial.begin(115200);
  
  pinMode(BT1, INPUT_PULLUP);
  pinMode(BT2, INPUT_PULLUP);

  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV2);

  pinMode(9, OUTPUT); 
  digitalWrite(9, LOW);

  pinMode(10, OUTPUT); 
  digitalWrite(10, HIGH);

}


void jammerLoop() {

  attachInterrupt(digitalPinToInterrupt(BT1), pressBt01, FALLING);
  attachInterrupt(digitalPinToInterrupt(BT2), pressBt02, FALLING);
  
 for (int i = 0; i <= 100; i++) {
  
  display.clearDisplay();
 if (!jamming) {
  display.setCursor(80, 20);
  display.println("disabled");
  pixels.setPixelColor(0, pixels.Color(0, 5, 0));
  pixels.show();
 }
 if (jamming) {
  display.setCursor(100, 20);
  display.print(i);
  display.println("%");
  pixels.setPixelColor(0, pixels.Color(5, 0, 0));
  pixels.show();
 }
    
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(100, 10);
  display.print("[");
  display.print(channel);
  display.println("]");

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("frequency");
  display.setCursor(90, 0);
  display.println("2.4GHz");

  display.setCursor(0, 10);
  display.println("channel");

  display.setCursor(0, 20);
  display.println("progress");
  
  display.display();
 }
  
  if (jamming) {
  Serial.println(("JAMMING CHANNEL" + String(channel + 1)));
  radioConfig();
  }
  while (jamming) {
    jammer();
  }
}
