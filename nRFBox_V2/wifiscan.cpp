/* ____________________________
   This software is licensed under the MIT License:
   https://github.com/cifertech/nrfbox
   ________________________________________ */

#include <Arduino.h> 
#include "wifiscan.h"

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

#define BTN_PIN_UP 26
#define BTN_PIN_DOWN 33
#define BTN_PIN_SELECT 27
#define BTN_PIN_BACK 25

int currentIndex = 0;
int listStartIndex = 0;
bool isDetailView = false;
unsigned long scan_StartTime = 0;
const unsigned long scanTimeout = 5000;
bool isScanComplete = false;

unsigned long lastButtonPress = 0;
unsigned long debounceTime = 200;

void wifiscanSetup() {
  Serial.begin(115200);
  u8g2.setFont(u8g2_font_6x10_tr);
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  
  pinMode(BTN_PIN_UP, INPUT_PULLUP);
  pinMode(BTN_PIN_DOWN, INPUT_PULLUP);
  pinMode(BTN_PIN_SELECT, INPUT_PULLUP);
  pinMode(BTN_PIN_BACK, INPUT_PULLUP);
  
  for (int cycle = 0; cycle < 3; cycle++) { 
    for (int i = 0; i < 3; i++) {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.drawStr(0, 10, "Scanning WiFi");

      String dots = "";
      for (int j = 0; j <= i; j++) {
        dots += " .";
        setNeoPixelColour("white");
      }
      setNeoPixelColour("0");
      
      u8g2.drawStr(80, 10, dots.c_str()); 

      u8g2.sendBuffer();
      delay(300); 
    }
  }
  
  scan_StartTime = millis();
  isScanComplete = false;
}

void wifiscanLoop() {
  unsigned long currentMillis = millis();

  if (!isScanComplete && currentMillis - scan_StartTime < scanTimeout) {
    int foundNetworks = WiFi.scanNetworks();
    if (foundNetworks >= 0) {
      isScanComplete = true;
    }
  }

  if (currentMillis - lastButtonPress > debounceTime) {
    if (digitalRead(BTN_PIN_UP) == LOW) {
      if (currentIndex > 0) {
        currentIndex--;
        if (currentIndex < listStartIndex) {
          listStartIndex--;
        }
      }
      lastButtonPress = currentMillis;
    } else if (digitalRead(BTN_PIN_DOWN) == LOW) {
      if (currentIndex < WiFi.scanComplete() - 1) {
        currentIndex++;
        if (currentIndex >= listStartIndex + 5) {
          listStartIndex++;
        }
      }
      lastButtonPress = currentMillis;
    } else if (digitalRead(BTN_PIN_SELECT) == LOW) {
      isDetailView = true;
      lastButtonPress = currentMillis;
    }
  }

  if (!isDetailView && isScanComplete) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(0, 10, "Wi-Fi Networks:");

    int networkCount = WiFi.scanComplete();
    for (int i = 0; i < 5; i++) {
      int currentNetworkIndex = i + listStartIndex;
      if (currentNetworkIndex >= networkCount) break;

      String networkName = WiFi.SSID(currentNetworkIndex);
      int rssi = WiFi.RSSI(currentNetworkIndex);

      String networkInfo = networkName.substring(0, 7);
      String networkrssi = " | RSSI " + String(rssi);

      if (currentNetworkIndex == currentIndex) {
        u8g2.drawStr(0, 20 + i * 10, ">");
      }
      u8g2.drawStr(10, 20 + i * 10, networkInfo.c_str());
      u8g2.drawStr(50, 20 + i * 10, networkrssi.c_str());
    }
    u8g2.sendBuffer();
  }

  if (isDetailView) {
    String networkName = WiFi.SSID(currentIndex);
    String networkBSSID = WiFi.BSSIDstr(currentIndex);
    int rssi = WiFi.RSSI(currentIndex);
    int channel = WiFi.channel(currentIndex);

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(0, 10, "Network Details:");

    u8g2.setFont(u8g2_font_5x8_tr);
    String name = "SSID: " + networkName;
    String bssid = "BSSID: " + networkBSSID;
    String signal = "RSSI: " + String(rssi);
    String ch = "Channel: " + String(channel);

    u8g2.drawStr(0, 20, name.c_str());
    u8g2.drawStr(0, 30, bssid.c_str());
    u8g2.drawStr(0, 40, signal.c_str());
    u8g2.drawStr(0, 50, ch.c_str());
    u8g2.drawStr(0, 60, "Press LEFT to go back");
    u8g2.sendBuffer();

    if (digitalRead(BTN_PIN_BACK) == LOW) {
      isDetailView = false;
      lastButtonPress = currentMillis;
    }
  }
}
