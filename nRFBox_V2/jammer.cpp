/* ____________________________
   This software is licensed under the MIT License:
   https://github.com/cifertech/nrfbox
   ________________________________________ */
   
#include <Arduino.h> 
#include "jammer.h"

#define BT1 33  // channels
#define BT2 27  // jamming
#define BT3 26  // data rate
#define BT4 25  // PA level

#define CE_PIN 5
#define CSN_PIN 17

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

RF24 radio(CE_PIN, CSN_PIN);

const int num_channels = 64;
int value[num_channels];
int valuesDisplay[32];
int channels = 1;
const int num_reps = 50;
bool jamming = false;
const byte address[6] = "00001";

uint8_t dataRateIndex = 0;  // Index for cycling through data rates
uint8_t paLevelIndex = 0;   // Index for cycling through PA levels

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;



void setRadioParameters() {
    switch (dataRateIndex) {
        case 0: radio.setDataRate(RF24_250KBPS); break;
        case 1: radio.setDataRate(RF24_1MBPS); break;
        case 2: radio.setDataRate(RF24_2MBPS); break;
    }
    
    switch (paLevelIndex) {
        case 0: radio.setPALevel(RF24_PA_MIN); break;
        case 1: radio.setPALevel(RF24_PA_LOW); break;
        case 2: radio.setPALevel(RF24_PA_HIGH); break;
        case 3: radio.setPALevel(RF24_PA_MAX); break;
    }

    Serial.print("Data Rate: ");
    Serial.println(dataRateIndex);
    Serial.print("PA Level: ");
    Serial.println(paLevelIndex);
}

void radioSetChannel(int channels) {
    radio.setChannel(channels);
}

void jammer() {
    const char text[] = "xxxxxxxxxxxxxxxx";
    for (int i = ((channels * 5) + 1); i < ((channels * 5) + 23); i++) {
        radioSetChannel(i);
        bool result = radio.write(&text, sizeof(text));
        if (result) {
            Serial.println("Transmission successful");
        } else {
            Serial.println("Transmission failed");
        }
        delay(10);
    }
}

void pressBt01() {
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();
    if (interrupt_time - last_interrupt_time > 200) {
        if (channels < 13) {
            channels++;
        } else {
            channels = 0;
        }
        Serial.print("Channel: ");
        Serial.println(channels);
    }
    last_interrupt_time = interrupt_time;
}

void pressBt02() {
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();
    if (interrupt_time - last_interrupt_time > 200) {
        jamming = !jamming;
        Serial.println(jamming ? "Jamming started" : "Jamming stopped");
    }
    last_interrupt_time = interrupt_time;
}

void pressBt03() {
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();
    if (interrupt_time - last_interrupt_time > 200) {
        dataRateIndex = (dataRateIndex + 1) % 3; // Cycle through data rates
        setRadioParameters();
        Serial.println("Data rate changed");
    }
    last_interrupt_time = interrupt_time;
}

void pressBt04() {
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();
    if (interrupt_time - last_interrupt_time > 200) {
        paLevelIndex = (paLevelIndex + 1) % 4; // Cycle through power levels
        setRadioParameters();
        Serial.println("Power level changed");
    }
    last_interrupt_time = interrupt_time;
}

void jammerSetup(){
    Serial.begin(115200);

    esp_bt_controller_deinit();
    esp_wifi_stop();
    esp_wifi_deinit();
    
    pinMode(BT1, INPUT_PULLUP);
    pinMode(BT2, INPUT_PULLUP);
    pinMode(BT3, INPUT_PULLUP);  
    pinMode(BT4, INPUT_PULLUP);  

    SPI.begin();
    
    pinMode(CE_PIN, OUTPUT);
    pinMode(CSN_PIN, OUTPUT);

    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.sendBuffer();

    radio.begin();
    setRadioParameters();
    radio.openWritingPipe(address);
    radio.stopListening();

    Serial.println("Radio configured and ready");  
}


void jammerLoop(){
    attachInterrupt(digitalPinToInterrupt(BT1), pressBt01, FALLING);
    attachInterrupt(digitalPinToInterrupt(BT2), pressBt02, FALLING);
    attachInterrupt(digitalPinToInterrupt(BT3), pressBt03, FALLING);
    attachInterrupt(digitalPinToInterrupt(BT4), pressBt04, FALLING);

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);

    // Display channel
    u8g2.setCursor(0, 10);
    u8g2.print("Channel : ");
    u8g2.setCursor(80, 10);
    u8g2.print("[");
    u8g2.print(channels);
    u8g2.print("]");

    // Display PA level
    u8g2.setCursor(0, 25);
    u8g2.print("PA Level: ");
    u8g2.setCursor(80, 25);
    switch (paLevelIndex) {
        case 0: u8g2.print("MIN"); break;
        case 1: u8g2.print("LOW"); break;
        case 2: u8g2.print("HIGH"); break;
        case 3: u8g2.print("MAX"); break;
    }

    // Display data rate
    u8g2.setCursor(0, 40);
    u8g2.print("Data Rate: ");
    u8g2.setCursor(80, 40);
    switch (dataRateIndex) {
        case 0: u8g2.print("250KBPS"); break;
        case 1: u8g2.print("1MBPS"); break;
        case 2: u8g2.print("2MBPS"); break;
    }

    u8g2.setCursor(0, 60);
    u8g2.print("Jamming: ");
 
    if (jamming) {
      u8g2.setCursor(80, 60);
      u8g2.print("Active ");        
    }else{
      u8g2.setCursor(80, 60);
      u8g2.print("disable ");
    }

    u8g2.sendBuffer();

    delay(50);

    if (jamming) {
      u8g2.setCursor(80, 60);
    u8g2.print("Active ");
        Serial.println("Starting jamming on channel " + String(channels + 1));
        jammer();
    }  
}
