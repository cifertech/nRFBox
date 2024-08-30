/* ____________________________
   This software is licensed under the MIT License:
   https://github.com/cifertech/nrfbox
   ________________________________________ */
   
#include <Arduino.h> 
#include "analyzer.h"

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

#define NRF24_CONFIG      0x00
#define NRF24_EN_AA       0x01
#define NRF24_RF_CH       0x05
#define NRF24_RF_SETUP    0x06
#define NRF24_RPD         0x09

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define N 128
uint8_t values[N];

#define CE  5
#define CSN 17

#define CHANNELS  64
int CHannel[CHANNELS];


byte getregister(byte r) {
  byte c;

  digitalWrite(CSN, LOW);
  SPI.transfer(r & 0x1F);
  c = SPI.transfer(0);
  digitalWrite(CSN, HIGH);

  return c;
}

void setregister(byte r, byte v) {
  digitalWrite(CSN, LOW);
  SPI.transfer((r & 0x1F) | 0x20);
  SPI.transfer(v);
  digitalWrite(CSN, HIGH);
}

void powerup(void) {
  setregister(NRF24_CONFIG, getregister(NRF24_CONFIG) | 0x02);
  delayMicroseconds(130);
}

void powerdown(void) {
  setregister(NRF24_CONFIG, getregister(NRF24_CONFIG) & ~0x02);
}

void ENable(void) {
  digitalWrite(CE, HIGH);
}

void DIsable(void) {
  digitalWrite(CE, LOW);
}

void setrx(void) {
  setregister(NRF24_CONFIG, getregister(NRF24_CONFIG) | 0x01);
  ENable();
  delayMicroseconds(100);
}

void ScanChannels(void) {
  DIsable();
 // for (int j = 0; j < 10; j++) {
    for (int i = 0; i < CHANNELS; i++) {
      setregister(NRF24_RF_CH, (128 * i) / CHANNELS);
      setrx();
      delayMicroseconds(40);
      DIsable();
      if (getregister(NRF24_RPD) > 0) CHannel[i]++;
    }
 // }
}


void writeRegister(uint8_t reg, uint8_t value) {
    digitalWrite(CSN, LOW);
    SPI.transfer(reg | 0x20);
    SPI.transfer(value);
    digitalWrite(CSN, HIGH);
}

uint8_t readRegister(uint8_t reg) {
    digitalWrite(CSN, LOW);
    SPI.transfer(reg & 0x1F);
    uint8_t result = SPI.transfer(0x00);
    digitalWrite(CSN, HIGH);
    return result;
}

void setChannel(uint8_t CHannel) {
    writeRegister(NRF24_RF_CH, CHannel);
}

void powerUP() {
    uint8_t config = readRegister(NRF24_CONFIG);
    writeRegister(NRF24_CONFIG, config | 0x02);
    delay(5);
}

void powerDOWN() {
    uint8_t config = readRegister(NRF24_CONFIG);
    writeRegister(NRF24_CONFIG, config & ~0x02);
}

void startListening() {
    digitalWrite(CE, HIGH);
}

void stopListening() {
    digitalWrite(CE, LOW);
}

bool carrierDetected() {
    return readRegister(NRF24_RPD) & 0x01;
}


void analyzerSetup(){
  
    Serial.begin(115200);

    esp_bt_controller_deinit();
    esp_wifi_stop();
    esp_wifi_deinit();
  
    pinMode(CE, OUTPUT);
    pinMode(CSN, OUTPUT);

    SPI.begin(18, 19, 23, 17);
    SPI.setDataMode(SPI_MODE0);
    SPI.setFrequency(10000000);
    SPI.setBitOrder(MSBFIRST);

    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.sendBuffer();

    digitalWrite(CSN, HIGH);
    digitalWrite(CE, LOW);

    DIsable();

    powerUP();
    writeRegister(NRF24_EN_AA, 0x00);
    writeRegister(NRF24_RF_SETUP, 0x0F);  

}

void analyzerLoop(){

    ScanChannels();

    memset(values, 0, sizeof(values));

    int n = 50;
    while (n--) {
        int i = N;
        while (i--) {
            setChannel(i);
            startListening();
            delayMicroseconds(128);
            stopListening();
            if (carrierDetected()) {
                ++values[i];
            }
        }
    }

    u8g2.clearBuffer();
    int barWidth = SCREEN_WIDTH / N;
    int x = 0;
    for (int i = 0; i < N; ++i) {
        int v = 63 - values[i] * 3;
        if (v < 0) {
            v = 0;
        }
        u8g2.drawVLine(x, v - 10, 64 - v);
        x += barWidth;
    }

    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.setCursor(0, 64);
    u8g2.print("1...5...10...25..50...80...128");
    u8g2.sendBuffer();

    delay(50);  

}
