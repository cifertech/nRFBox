/* ____________________________
   This software is licensed under the MIT License:
   https://github.com/cifertech/nrfbox
   ________________________________________ */

#include <Arduino.h>
#include "blackout.h"
#include "icon.h"

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

#define CE_PIN_1  5
#define CSN_PIN_1 17

#define CE_PIN_2  16
#define CSN_PIN_2 4

#define CE_PIN_3  15
#define CSN_PIN_3 2

#define MODE_BUTTON 25
#define MODE_BUTTON1 27
#define MODE_BUTTON2 26

RF24 radio_1(CE_PIN_1, CSN_PIN_1, 16000000);
RF24 radio_2(CE_PIN_2, CSN_PIN_2, 16000000);
RF24 radio_3(CE_PIN_3, CSN_PIN_3, 16000000);

enum OperationMode {WiFi_MODULE, VIDEO_TX_MODULE, RC_MODULE, BLE_MODULE, Bluetooth_MODULE, USB_WIRELESS_MODULE, ZIGBEE_MODULE, NRF24_MODULE};
OperationMode current_Mode = WiFi_MODULE;

enum Operation {DEACTIVE_MODE, ACTIVE_MODE };
Operation current = DEACTIVE_MODE;


// Channel groups
byte channelGroup_1[] = {2, 5, 8, 11};
byte channelGroup_2[] = {26, 29, 32, 35};
byte channelGroup_3[] = {80, 83, 86, 89};

// Protocol channel definitions
const byte bluetooth_channels[] =        {32, 34, 46, 48, 50, 52, 0, 1, 2, 4, 6, 8, 22, 24, 26, 28, 30, 74, 76, 78, 80};
const byte ble_channels[] =              {2, 26, 80};
const byte WiFi_channels[] =             {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
const byte usbWireless_channels[] =      {40, 50, 60};
const byte videoTransmitter_channels[] = {70, 75, 80};
const byte rc_channels[] =               {1, 3, 5, 7};
const byte zigbee_channels[] =           {11, 15, 20, 25};
const byte nrf24_channels[] =            {76, 78, 79};

volatile bool ChangeRequested = false;
volatile bool ChangeRequested1 = false;
volatile bool ChangeRequested2 = false;

unsigned long lastPressTime = 0;
const unsigned long debounceDelay = 200;


void IRAM_ATTR handleButton() {
  unsigned long currentTime = millis();
  if (currentTime - lastPressTime > debounceDelay) {
    ChangeRequested = true;
    lastPressTime = currentTime;
  }
}

void IRAM_ATTR handleButton1() {
  unsigned long currentTime = millis();
  if (currentTime - lastPressTime > debounceDelay) {
    ChangeRequested1 = true;
    lastPressTime = currentTime;
  }
}

void IRAM_ATTR handleButton2() {
  unsigned long currentTime = millis();
  if (currentTime - lastPressTime > debounceDelay) {
    if (current == DEACTIVE_MODE) {
      current = ACTIVE_MODE;  
    } else {
      current = DEACTIVE_MODE;  
    }
    lastPressTime = currentTime;  
  }
}

void configure_Radio(RF24 &radio, const byte *channels, size_t size) {
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

void initialize_MultiMode() {
  if (radio_1.begin()) {
    configure_Radio(radio_1, channelGroup_1, sizeof(channelGroup_1));
  }
  if (radio_2.begin()) {
    configure_Radio(radio_2, channelGroup_2, sizeof(channelGroup_2));
  }
  if (radio_3.begin()) {
    configure_Radio(radio_3, channelGroup_3, sizeof(channelGroup_3));
  }
}

void initialize_Radios() {
  if (current == ACTIVE_MODE) {
    initialize_MultiMode();
  } else if (current == DEACTIVE_MODE) {
    radio_1.powerDown();
    radio_2.powerDown();
    radio_3.powerDown();
    delay(100);  
  }
}

void update_OLED() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x8_tr);

  u8g2.setCursor(75, 7);
  if (current == ACTIVE_MODE) {
     u8g2.print("-ACTIVE-");
  } else if (current == DEACTIVE_MODE) {
     u8g2.print("-DEACTIVE-");  
  }
  
  u8g2.setCursor(0, 12);
  u8g2.println("-------------------------------");
  u8g2.setCursor(0, 7);
  //u8g2.print("Mode: ");
  u8g2.print("[");

  u8g2.drawXBMP(0, 5, 128, 64, bitmap_arrow_left);
  u8g2.drawXBMP(0, 5, 128, 64, bitmap_arrow_right);
  
  switch (current_Mode) {
    case WiFi_MODULE:
        u8g2.drawXBMP(0, 5, 128, 64, bitmap_wifi_jammer);
        u8g2.print("WiFi");
        break;
    case VIDEO_TX_MODULE:
        u8g2.drawXBMP(0, 5, 128, 64, bitmap_cctv);
        u8g2.print("Video TX");
        break;
    case RC_MODULE:
        u8g2.drawXBMP(0, 5, 128, 64, bitmap_rc);
        u8g2.print("RC");
        break;
    case BLE_MODULE:
        u8g2.drawXBMP(0, 5, 128, 64, bitmap_ble_jammer);
        u8g2.print("BLE");
        break;
    case Bluetooth_MODULE:
        u8g2.drawXBMP(0, 5, 128, 64, bitmap_bluetooth_jammer);
        u8g2.print("Bluetooth");
        break;
    case USB_WIRELESS_MODULE:
        u8g2.drawXBMP(0, 5, 128, 64, bitmap_usb);
        u8g2.print("USB Wireless");
        break;
    case ZIGBEE_MODULE:
        u8g2.drawXBMP(0, 5, 128, 64, bitmap_zigbee);
        u8g2.print("Zigbee");
        break;
    case NRF24_MODULE:
        u8g2.drawXBMP(0, 5, 128, 64, bitmap_nrf24);
        u8g2.print("NRF24");
        break;
 }
  u8g2.print("]");
  u8g2.sendBuffer();
}

void checkMode() {
  if (ChangeRequested) {
    ChangeRequested = false;
    current_Mode = static_cast<OperationMode>((current_Mode == 0) ? 7 : (current_Mode - 1));
    update_OLED();
  } else if (ChangeRequested1) {
    ChangeRequested1 = false;
    current_Mode = static_cast<OperationMode>((current_Mode + 1) % 8);     
    update_OLED();
  }
}

void blackoutSetup() {
  Serial.begin(115200);

  esp_bt_controller_deinit();
  esp_wifi_stop();
  esp_wifi_deinit();
  esp_wifi_disconnect();

  pinMode(MODE_BUTTON, INPUT_PULLUP);
  pinMode(MODE_BUTTON1, INPUT_PULLUP);
  pinMode(MODE_BUTTON2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(MODE_BUTTON), handleButton, FALLING);
  attachInterrupt(digitalPinToInterrupt(MODE_BUTTON1), handleButton1, FALLING);
  attachInterrupt(digitalPinToInterrupt(MODE_BUTTON2), handleButton2, FALLING);

  initialize_Radios();
  update_OLED();
}

void blackoutLoop() {
  checkMode();

  static Operation lastMode = DEACTIVE_MODE;
  
  if (current != lastMode) {
    lastMode = current;
    initialize_Radios();
    update_OLED();
  }

  if (current_Mode == BLE_MODULE) {
        int randomIndex = random(0, sizeof(ble_channels) / sizeof(ble_channels[0]));
        int channel = ble_channels[randomIndex];
        radio_1.setChannel(channel);
        radio_2.setChannel(channel);
        radio_3.setChannel(channel);
        
  } else if (current_Mode == Bluetooth_MODULE) {
        int randomIndex = random(0, sizeof(bluetooth_channels) / sizeof(bluetooth_channels[0]));
        int channel = bluetooth_channels[randomIndex];
        radio_1.setChannel(channel);
        radio_2.setChannel(channel);
        radio_3.setChannel(channel);
        
  }else if (current_Mode == WiFi_MODULE) {
        int randomIndex = random(0, sizeof(WiFi_channels) / sizeof(WiFi_channels[0]));
        int channel = WiFi_channels[randomIndex];
        radio_1.setChannel(channel);
        radio_2.setChannel(channel);
        radio_3.setChannel(channel);
        
  }else if (current_Mode == USB_WIRELESS_MODULE) {
        int randomIndex = random(0, sizeof(usbWireless_channels) / sizeof(usbWireless_channels[0]));
        int channel = usbWireless_channels[randomIndex];
        radio_1.setChannel(channel);
        radio_2.setChannel(channel);
        radio_3.setChannel(channel);
        
  }else if (current_Mode == VIDEO_TX_MODULE) {
        int randomIndex = random(0, sizeof(videoTransmitter_channels) / sizeof(videoTransmitter_channels[0]));
        int channel = videoTransmitter_channels[randomIndex];
        radio_1.setChannel(channel);
        radio_2.setChannel(channel);
        radio_3.setChannel(channel);
        
  }else if (current_Mode == RC_MODULE) {
        int randomIndex = random(0, sizeof(rc_channels) / sizeof(rc_channels[0]));
        int channel = rc_channels[randomIndex];
        radio_1.setChannel(channel);
        radio_2.setChannel(channel);
        radio_3.setChannel(channel);
        
  }else if (current_Mode == ZIGBEE_MODULE) {
        int randomIndex = random(0, sizeof(zigbee_channels) / sizeof(zigbee_channels[0]));
        int channel = zigbee_channels[randomIndex];
        radio_1.setChannel(channel);
        radio_2.setChannel(channel);
        radio_3.setChannel(channel);     
        
  }else if (current_Mode == NRF24_MODULE) {
        int randomIndex = random(0, sizeof(nrf24_channels) / sizeof(nrf24_channels[0]));
        int channel = nrf24_channels[randomIndex];
        radio_1.setChannel(channel);
        radio_2.setChannel(channel);
        radio_3.setChannel(channel);
  } 
}
