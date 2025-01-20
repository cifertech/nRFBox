/* ____________________________
   This software is licensed under the MIT License:
   https://github.com/cifertech/nrfbox
   ________________________________________ */

#include <Arduino.h> 
#include "blejammer.h"

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

#define CE_PIN_1  5
#define CSN_PIN_1 17

#define CE_PIN_2  16
#define CSN_PIN_2 4

#define CE_PIN_3  15
#define CSN_PIN_3 2

#define MODE_BUTTON 26

RF24 radio1(CE_PIN_1, CSN_PIN_1, 16000000);
RF24 radio2(CE_PIN_2, CSN_PIN_2, 16000000);
RF24 radio3(CE_PIN_3, CSN_PIN_3, 16000000);

enum OperationMode { DEACTIVE_MODE, BLE_MODULE, Bluetooth_MODULE };
OperationMode currentMode = DEACTIVE_MODE;

int bluetooth_channels[] = {32, 34, 46, 48, 50, 52, 0, 1, 2, 4, 6, 8, 22, 24, 26, 28, 30, 74, 76, 78, 80};
int ble_channels[] = {2, 26, 80};

const byte BLE_channels[] = {2, 26, 80}; 
byte channelGroup1[] = {2, 5, 8, 11};    
byte channelGroup2[] = {26, 29, 32, 35}; 
byte channelGroup3[] = {80, 83, 86, 89}; 

volatile bool modeChangeRequested = false;

unsigned long lastJammingTime = 0;
const unsigned long jammingInterval = 10;

unsigned long lastButtonPressTime = 0;
const unsigned long debounceDelay = 500;

void IRAM_ATTR handleButtonPress() {
  unsigned long currentTime = millis();
  if (currentTime - lastButtonPressTime > debounceDelay) {
    modeChangeRequested = true;
    lastButtonPressTime = currentTime;
  }
}

void configureRadio(RF24 &radio, const byte* channels, size_t size) {
  radio.setAutoAck(false);
  radio.stopListening();
  radio.setRetries(0, 0);
  radio.setPALevel(RF24_PA_MAX, true);
  radio.setDataRate(RF24_2MBPS);
  radio.setCRCLength(RF24_CRC_DISABLED);
  radio.printPrettyDetails();

  for (size_t i = 0; i < size; i++) {
    radio.setChannel(channels[i]);
    radio.startConstCarrier(RF24_PA_MAX, channels[i]);
  }
}

void initializeRadiosMultiMode() {
  if (radio1.begin()) {
    configureRadio(radio1, channelGroup1, sizeof(channelGroup1));
  }
  if (radio2.begin()) {
    configureRadio(radio2, channelGroup2, sizeof(channelGroup2));
  }
  if (radio3.begin()) {
    configureRadio(radio3, channelGroup3, sizeof(channelGroup3));
  }
}

void initializeRadios() {
  if (currentMode == !DEACTIVE_MODE) {
    initializeRadiosMultiMode();
  } else if (currentMode == DEACTIVE_MODE) {
    radio1.powerDown();
    radio2.powerDown();
    radio3.powerDown();
    delay(100);
  } 
}

void jammer(RF24 &radio, const byte* channels, size_t size) {
  const char text[] = "xxxxxxxxxxxxxxxx";
  for (size_t i = 0; i < size; i++) {
    radio.setChannel(channels[i]);
    radio.write(&text, sizeof(text));
    //delayMicroseconds(20);
  }
}

void updateOLED() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);

  u8g2.setCursor(0, 10);
  u8g2.print("Mode ");
  u8g2.print(" ....... ");
  u8g2.setCursor(65, 10);
  u8g2.print("[");
  u8g2.print(currentMode == BLE_MODULE ? "BLE" : currentMode == Bluetooth_MODULE ? "Bluetooth" : "Deactive");
  u8g2.print("]");

  u8g2.setCursor(0, 35);
  u8g2.print("Radio 1: ");
  u8g2.setCursor(70, 35);
  u8g2.print(radio1.isChipConnected() ? "Active" : "Inactive");

  u8g2.setCursor(0, 50);
  u8g2.print("Radio 2: ");
  u8g2.setCursor(70, 50);
  u8g2.print(radio2.isChipConnected() ? "Active" : "Inactive");

  u8g2.setCursor(0, 64);
  u8g2.print("Radio 3: ");
  u8g2.setCursor(70, 64);
  u8g2.print(radio3.isChipConnected() ? "Active" : "Inactive");

  u8g2.sendBuffer();
}

void checkModeChange() {
  if (modeChangeRequested) {
    modeChangeRequested = false;
    currentMode = static_cast<OperationMode>((currentMode + 1) % 3);
    initializeRadios();
    updateOLED();
  }
}

void blejammerSetup() {
  Serial.begin(115200);

  esp_bt_controller_deinit();
  esp_wifi_stop();
  esp_wifi_deinit();
  esp_wifi_disconnect();

  pinMode(MODE_BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(MODE_BUTTON), handleButtonPress, FALLING);

  initializeRadios();
  updateOLED();
}

void blejammerLoop() {
  checkModeChange();

  if (currentMode == BLE_MODULE) {
        int randomIndex = random(0, sizeof(ble_channels) / sizeof(ble_channels[0]));
        int channel = ble_channels[randomIndex];
        radio1.setChannel(channel);
        radio2.setChannel(channel);
        radio3.setChannel(channel);
        
  } else if (currentMode == Bluetooth_MODULE) {
        int randomIndex = random(0, sizeof(bluetooth_channels) / sizeof(bluetooth_channels[0]));
        int channel = bluetooth_channels[randomIndex];
        radio1.setChannel(channel);
        radio2.setChannel(channel);
        radio3.setChannel(channel);
  }
}
