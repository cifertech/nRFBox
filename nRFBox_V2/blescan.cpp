/* ____________________________
   This software is licensed under the MIT License:
   https://github.com/cifertech/nrfbox
   ________________________________________ */

#include <Arduino.h> 
#include "blescan.h"

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

#define BUTTON_PIN_UP 26
#define BUTTON_PIN_DOWN 33
#define BUTTON_PIN_SELECT 27
#define BUTTON_PIN_BACK 25

BLEScan* scan;
BLEScanResults results;

int selectedIndex = 0;
int displayStartIndex = 0;
bool showDetails = false;
unsigned long scanStartTime = 0;
const unsigned long scanDuration = 5000;
bool scanComplete = false;

unsigned long lastDebounce = 0;
unsigned long debounce_Delay = 200;

void blescanSetup() {
  Serial.begin(115200);
  u8g2.setFont(u8g2_font_6x10_tr);
  
  BLEDevice::init("");
  scan = BLEDevice::getScan();
  scan->setActiveScan(true);
  
  pinMode(BUTTON_PIN_UP, INPUT_PULLUP);
  pinMode(BUTTON_PIN_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_PIN_SELECT, INPUT_PULLUP);
  pinMode(BUTTON_PIN_BACK, INPUT_PULLUP);

  
  for (int cycle = 0; cycle < 3; cycle++) { 
    for (int i = 0; i < 3; i++) {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.drawStr(0, 10, "Scanning BLE");

      String dots = "";
      for (int j = 0; j <= i; j++) {
        dots += " .";
        setNeoPixelColour("white");
      }
      setNeoPixelColour("0");
      
      u8g2.drawStr(75, 10, dots.c_str()); 

      u8g2.sendBuffer();
      delay(300); 
    }
  }

  scanStartTime = millis();
  scan->start(scanDuration / 1000, false);
}

void blescanLoop() {
  unsigned long currentMillis = millis();
  if (currentMillis - scanStartTime >= scanDuration && !scanComplete) {
    scanComplete = true;
    results = scan->getResults();
    scan->stop();
    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "Scan complete.");
    u8g2.sendBuffer();
  }

  if (currentMillis - lastDebounce > debounce_Delay) {
    if (digitalRead(BUTTON_PIN_UP) == LOW) {
      if (selectedIndex > 0) {
        selectedIndex--;
        if (selectedIndex < displayStartIndex) {
          displayStartIndex--;
        }
      }
      lastDebounce = currentMillis;
    } else if (digitalRead(BUTTON_PIN_DOWN) == LOW) {
      if (selectedIndex < results.getCount() - 1) {
        selectedIndex++;
        if (selectedIndex >= displayStartIndex + 5) {
          displayStartIndex++;
        }
      }
      lastDebounce = currentMillis;
    } else if (digitalRead(BUTTON_PIN_SELECT) == LOW) {
      showDetails = true;
      lastDebounce = currentMillis;
    }
  }

  if (!showDetails && scanComplete) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(0, 10, "BLE Devices:");

    int deviceCount = results.getCount();
    for (int i = 0; i < 5; i++) {
      int deviceIndex = i + displayStartIndex;
      if (deviceIndex >= deviceCount) break;
      BLEAdvertisedDevice device = results.getDevice(deviceIndex);
      String deviceName = device.getName().c_str();
      if (deviceName.length() == 0) {
        deviceName = "No Name";
      }
      String deviceInfo = deviceName.substring(0, 7) + " | RSSI " + String(device.getRSSI());
      if (deviceIndex == selectedIndex) {
        u8g2.drawStr(0, 20 + i * 10, ">");
      }
      u8g2.drawStr(10, 20 + i * 10, deviceInfo.c_str());
    }
    u8g2.sendBuffer();
  }

  if (showDetails) {
    BLEAdvertisedDevice device = results.getDevice(selectedIndex);
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(0, 10, "Device Details:");
    u8g2.setFont(u8g2_font_5x8_tr);
    String name = "Name: " + String(device.getName().c_str());
    String address = "Addr: " + String(device.getAddress().toString().c_str());
    String rssi = "RSSI: " + String(device.getRSSI());
    u8g2.drawStr(0, 20, name.c_str());
    u8g2.drawStr(0, 30, address.c_str());
    u8g2.drawStr(0, 40, rssi.c_str());
    u8g2.drawStr(0, 50, "Press LEFT to go back");
    u8g2.sendBuffer();

    if (digitalRead(BUTTON_PIN_BACK) == LOW) {
      showDetails = false;
      lastDebounce = currentMillis;
    }
  }  
}
