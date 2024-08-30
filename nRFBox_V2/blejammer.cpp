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

RF24 radio1(CE_PIN_1, CSN_PIN_1);
RF24 radio2(CE_PIN_2, CSN_PIN_2);
RF24 radio3(CE_PIN_3, CSN_PIN_3);

enum OperationMode { DEACTIVE_MODE, SINGLE_MODULE, MULTI_MODULE };
OperationMode currentMode = DEACTIVE_MODE;

const byte channels[] = {2, 26, 80};
byte currentChannelIndex = 0;

volatile bool modeChangeRequested = false;

unsigned long lastJammingTime = 0;
const unsigned long jammingInterval = 10;

unsigned long lastButtonPressTime = 0;
const unsigned long debounceDelay = 50;


void IRAM_ATTR handleButtonPress() {
  unsigned long currentTime = millis();
  if (currentTime - lastButtonPressTime > debounceDelay) {
    modeChangeRequested = true;
    lastButtonPressTime = currentTime;
  }
}

void configureRadio(RF24 &radio, byte channel) {
  radio.powerDown();
  delay(1000);
  radio.powerUp();
  radio.setAutoAck(false);
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.stopListening();
  radio.setChannel(channel);
}

void initializeRadiosMultiMode() {
  if (radio1.begin()) {
    Serial.println("Multi-Module Mode: Radio 1 started");
    configureRadio(radio1, 80);
  } else {
    Serial.println("Failed to initialize Radio 1");
  }

  if (radio2.begin()) {
    Serial.println("Multi-Module Mode: Radio 2 started");
    configureRadio(radio2, 26);
  } else {
    Serial.println("Failed to initialize Radio 2");
  }

  if (radio3.begin()) {
    Serial.println("Multi-Module Mode: Radio 3 started");
    configureRadio(radio3, 2);
  } else {
    Serial.println("Failed to initialize Radio 3");
  }

  Serial.println("All radios configured and ready in Multi-Module Mode");
}

void initializeRadios() {
  if (currentMode == SINGLE_MODULE) {
    if (radio1.begin()) {
      Serial.println("Single Module Mode: Radio 1 started");
      configureRadio(radio1, channels[currentChannelIndex]);
    } else {
      Serial.println("Failed to initialize Radio 1 in Single Module Mode");
    }
  } else if (currentMode == MULTI_MODULE) {
    initializeRadiosMultiMode();
  } else if (currentMode == DEACTIVE_MODE) {
    Serial.println("Deactive Mode: All radios are turned off");
    radio1.powerDown();
    radio2.powerDown();
    radio3.powerDown();
  }
}

void jammer(RF24 &radio, int channel) {
  const char text[] = "xxxxxxxxxxxxxxxx";
  for (int i = (channel * 5) + 1; i < (channel * 5) + 23; i++) {
    radio.setChannel(i);
    bool result = radio.write(&text, sizeof(text));
    if (result) {
      Serial.println("Transmission successful");
    } else {
      Serial.println("Transmission failed");
    }
    delay(10);
  }
}

void updateOLED() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);

  u8g2.setCursor(0, 10);
  u8g2.print("Mode ");
  u8g2.print(" ........ ");
  u8g2.setCursor(70, 10);
  u8g2.print("[");
  u8g2.print(currentMode == SINGLE_MODULE ? "Single" : currentMode == MULTI_MODULE ? "Multi" : "Deactive");
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
    Serial.println(currentMode == SINGLE_MODULE ? "Switched to Single Module Mode" :
                   currentMode == MULTI_MODULE ? "Switched to Multi-Module Mode" :
                   "Switched to Deactive Mode");

    initializeRadios();
    updateOLED();
  }
}


void blejammerSetup(){
  
  Serial.begin(115200);
  
  esp_bt_controller_deinit();
  esp_wifi_stop();
  esp_wifi_deinit();
  u8g2.begin();
  pinMode(MODE_BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(MODE_BUTTON), handleButtonPress, FALLING);

  initializeRadios();
  updateOLED();
 
}

void blejammerLoop(){
  
  checkModeChange();

  if (currentMode == SINGLE_MODULE) {
    if (millis() - lastJammingTime >= jammingInterval) {
      jammer(radio1, channels[currentChannelIndex]);
      currentChannelIndex = (currentChannelIndex + 1) % 3;
      Serial.print("Single Module Mode: Jamming on channel range ");
      Serial.println(channels[currentChannelIndex]);
      lastJammingTime = millis();
    }
  } else if (currentMode == MULTI_MODULE) {
    if (millis() - lastJammingTime >= jammingInterval) {
      jammer(radio1, 80);
      jammer(radio2, 26);
      jammer(radio3, 2);

      Serial.println("Multi-Module Mode: Jamming on all channel ranges");

      lastJammingTime = millis();
      updateOLED();
    }
  }  
}
