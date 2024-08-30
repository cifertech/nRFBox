/* ____________________________
   This software is licensed under the MIT License:
   https://github.com/cifertech/nrfbox
   ________________________________________ */
   
#include <Arduino.h> 
#include "scanner.h"

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern Adafruit_NeoPixel pixels;

#define CE  5
#define CSN 17

#define BUTTON 26

#define CHANNELS  64
int channel[CHANNELS];

int line;
char grey[] = " .:-=+*aRW";

#define _NRF24_CONFIG      0x00
#define _NRF24_EN_AA       0x01
#define _NRF24_RF_CH       0x05
#define _NRF24_RF_SETUP    0x06
#define _NRF24_RPD         0x09

char filled = 'F'; 
char drawDirection = 'R'; 
char slope = 'W'; 

byte sensorArray[129];

byte getRegister(byte r) {
  byte c;

  digitalWrite(CSN, LOW);
  SPI.transfer(r & 0x1F);
  c = SPI.transfer(0);
  digitalWrite(CSN, HIGH);

  return c;
}

void setRegister(byte r, byte v) {
  digitalWrite(CSN, LOW);
  SPI.transfer((r & 0x1F) | 0x20);
  SPI.transfer(v);
  digitalWrite(CSN, HIGH);
}

void powerUp(void) {
  setRegister(_NRF24_CONFIG, getRegister(_NRF24_CONFIG) | 0x02);
  delayMicroseconds(130);
}

void powerDown(void) {
  setRegister(_NRF24_CONFIG, getRegister(_NRF24_CONFIG) & ~0x02);
}

void enable(void) {
  digitalWrite(CE, HIGH);
}

void disable(void) {
  digitalWrite(CE, LOW);
}

void setRX(void) {
  setRegister(_NRF24_CONFIG, getRegister(_NRF24_CONFIG) | 0x01);
  enable();
  delayMicroseconds(100);
}

void scanChannels(void) {
  disable();
  for (int j = 0; j < 55; j++) {
    for (int i = 0; i < CHANNELS; i++) {
      setRegister(_NRF24_RF_CH, (128 * i) / CHANNELS);
      setRX();
      delayMicroseconds(40);
      disable();
      if (getRegister(_NRF24_RPD) > 0) channel[i]++;
    }
  }
}

void outputChannels(void) {
  int norm = 0;

  for (int i = 0; i < CHANNELS; i++)
    if (channel[i] > norm) norm = channel[i];
  
  Serial.print('|');
  for (int i = 0; i < CHANNELS; i++) {
    int pos;
    if (norm != 0) pos = (channel[i] * 10) / norm;
    else pos = 0;
    if (pos == 0 && channel[i] > 0) pos++;
    if (pos > 9) pos = 9;
    Serial.print(grey[pos]);
    channel[i] = 0;
  }

  Serial.print("| ");
  Serial.println(norm);

  u8g2.clearBuffer();

  u8g2.drawLine(0, 0, 0, 63);
  u8g2.drawLine(127, 0, 127, 63);

  for (byte count = 0; count < 64; count += 10) {
    u8g2.drawLine(127, count, 122, count);
    u8g2.drawLine(0, count, 5, count);
  }

  for (byte count = 10; count < 127; count += 10) {
    u8g2.drawPixel(count, 0);
    u8g2.drawPixel(count, 63);
  }

  if (norm < 10){
    byte drawHeight = map(norm, 0, 20, 0, 64);
    sensorArray[0] = drawHeight;
  }
  if (norm > 10){
    byte drawHeight = map(norm, 0, 40, 0, 64);
    sensorArray[0] = drawHeight;
  }

  
  

  for (byte count = 1; count <= 127; count++) {
    if (filled == 'D' || filled == 'd') {
      if (drawDirection == 'L' || drawDirection == 'l') {
        u8g2.drawPixel(count, 63 - sensorArray[count - 1]);
      } else {
        u8g2.drawPixel(127 - count, 63 - sensorArray[count - 1]);
      }
    } else {
      if (drawDirection == 'L' || drawDirection == 'l') {
        if (slope == 'W' || slope == 'w') {
          u8g2.drawLine(count, 63, count, 63 - sensorArray[count - 1]);
        } else {
          u8g2.drawLine(count, 0, count, 63 - sensorArray[count - 1]);
        }
      } else {
        if (slope == 'W' || slope == 'w') {
          u8g2.drawLine(127 - count, 63, 127 - count, 63 - sensorArray[count - 1]);
        } else {
          u8g2.drawLine(127 - count, 0, 127 - count, 63 - sensorArray[count - 1]);
        }
      }
    }
  }

  u8g2.setFont(u8g2_font_ncenB08_tr); 
  u8g2.setCursor(12, 12);
  u8g2.print(norm);

  u8g2.sendBuffer();

  for (byte count = 127; count >= 2; count--) {
    sensorArray[count - 1] = sensorArray[count - 2];
  }
}

void checkButtons() {
  static unsigned long lastDebounceTime1 = 0;
  static int lastButtonState1 = LOW;

  int buttonState1 = digitalRead(BUTTON);

  if (buttonState1 != lastButtonState1) {
    lastDebounceTime1 = millis();
  }
  if ((millis() - lastDebounceTime1) > 50) {
    if (buttonState1 == LOW) {
      if (filled == 'F') filled = 'D';
      else filled = 'F';
      Serial.println(filled); 
    }
  }
  lastButtonState1 = buttonState1;
}


void scannerSetup(){
  Serial.begin(115200);

  esp_bt_controller_deinit();
  esp_wifi_stop();
  esp_wifi_deinit();

  u8g2.begin();
  
  for (byte count = 0; count <= 128; count++) {
    sensorArray[count] = 0;
  }

  Serial.println("Starting 2.4GHz Scanner ...");
  Serial.println();

  SPI.begin(18, 19, 23, 17);
  SPI.setDataMode(SPI_MODE0);
  SPI.setFrequency(10000000);
  SPI.setBitOrder(MSBFIRST);

  pinMode(CE, OUTPUT);
  pinMode(CSN, OUTPUT);

  pinMode(BUTTON, INPUT_PULLUP);

  disable();

  powerUp();
  setRegister(_NRF24_EN_AA, 0x0);
  setRegister(_NRF24_RF_SETUP, 0x0F);
}

void scannerLoop(){

  checkButtons();
  scanChannels();
  outputChannels();
  
}
