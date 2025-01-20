/* ____________________________
   This software is licensed under the MIT License:
   https://github.com/cifertech/nrfbox
   ________________________________________ */

#include <EEPROM.h> // Include EEPROM library
#include <Arduino.h> 
#include "scanner.h"

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern Adafruit_NeoPixel pixels;

#define CE  5
#define CSN 17

#define CHANNELS  64
int channel[CHANNELS];

int line;
char grey[] = " .:-=+*aRW";

#define _NRF24_CONFIG      0x00
#define _NRF24_EN_AA       0x01
#define _NRF24_RF_CH       0x05
#define _NRF24_RF_SETUP    0x06
#define _NRF24_RPD         0x09

#define EEPROM_ADDRESS_SENSOR_ARRAY 2 

byte sensorArray[129];

unsigned long lastSaveTime = 0; 
const unsigned long saveInterval = 5000; 

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

  memset(channel, 0, sizeof(channel));

  const int samplesPerChannel = 50; // Number of samples per channel to average

  for (int i = 0; i < CHANNELS; i++) {
    setRegister(_NRF24_RF_CH, (128 * i) / CHANNELS);

    for (int j = 0; j < samplesPerChannel; j++) {
      setRX();
      delayMicroseconds(100); 
      disable();
      channel[i] += getRegister(_NRF24_RPD); // Add the RPD value (1 or 0)
    }

    // Average the accumulated values for this channel
    channel[i] = (channel[i] * 100) / samplesPerChannel; // Convert to percentage
  }
}


void outputChannels(void) {
  int norm = 0;

  // Find the maximum value in the channel array for normalization
  for (int i = 0; i < CHANNELS; i++) {
    if (channel[i] > norm) {
      norm = channel[i];
    }
  }

  byte drawHeight = map(norm, 0, 64, 0, 64); 
  
  // Update sensorArray with the new value (shift left for right-to-left movement)
  for (byte count = 126; count > 0; count--) {
    sensorArray[count] = sensorArray[count - 1];
  }
  sensorArray[0] = drawHeight;

  u8g2.clearBuffer();

  u8g2.drawLine(0, 0, 0, 63);
  u8g2.drawLine(127, 0, 127, 63);

  for (byte count = 0; count < 64; count += 10) {
    u8g2.drawLine(127, count, 122, count); // Right side markers
    u8g2.drawLine(0, count, 5, count);    // Left side markers
  }

  for (byte count = 10; count < 127; count += 10) {
    u8g2.drawPixel(count, 0);
    u8g2.drawPixel(count, 63);
  }

  // Draw the graph moving right-to-left
  for (byte count = 0; count < 127; count++) {
    u8g2.drawLine(127 - count, 63, 127 - count, 63 - sensorArray[count]);
    setNeoPixelColour("purple");
  }
  
  setNeoPixelColour("0");

  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.setCursor(12, 12);
  u8g2.print("[");
  u8g2.print(norm);
  u8g2.print("]");

  u8g2.sendBuffer();
}

void loadPreviousGraph() {
  EEPROM.begin(128); 
  for (byte i = 0; i < 128; i++) {
    sensorArray[i] = EEPROM.read(EEPROM_ADDRESS_SENSOR_ARRAY + i);
  }
  EEPROM.end(); 
}

void saveGraphToEEPROM() {
  EEPROM.begin(128); 
  for (byte i = 0; i < 128; i++) {
    EEPROM.write(EEPROM_ADDRESS_SENSOR_ARRAY + i, sensorArray[i]);
  }
  EEPROM.commit(); 
  EEPROM.end();    
}

void scannerSetup() {
  Serial.begin(115200);

  esp_bt_controller_deinit();
  esp_wifi_stop();
  esp_wifi_deinit();
  
  for (byte count = 0; count <= 128; count++) {
    sensorArray[count] = 0;
  }

  SPI.begin(18, 19, 23, 17);
  SPI.setDataMode(SPI_MODE0);
  SPI.setFrequency(16000000);
  SPI.setBitOrder(MSBFIRST);

  pinMode(CE, OUTPUT);
  pinMode(CSN, OUTPUT);

  disable();

  powerUp();
  setRegister(_NRF24_EN_AA, 0x0);
  setRegister(_NRF24_RF_SETUP, 0x0F);

  loadPreviousGraph();
}

void scannerLoop() {
  scanChannels();
  outputChannels();

  // Save the graph to EEPROM every 5 seconds
  if (millis() - lastSaveTime > saveInterval) {
    saveGraphToEEPROM();
    lastSaveTime = millis();
  }
}
