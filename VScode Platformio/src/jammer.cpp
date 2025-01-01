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


#define CE_A  5
#define CSN_A 17

#define CE_B  16
#define CSN_B 4

#define CE_C  15
#define CSN_C 2

RF24 radioA(CE_A, CSN_A, 16000000);
RF24 radioB(CE_B, CSN_B, 16000000);
RF24 radioC(CE_C, CSN_C, 16000000);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64


const int     num_channels = 64;
int           value[num_channels];
int           valuesDisplay[32];
int           channels = 1;
const int     num_reps = 50;
bool          jamming = false;
const byte    address[6] = "00001";

uint8_t dataRateIndex = 0;  // Index for cycling through data rates
uint8_t paLevelIndex = 0;   // Index for cycling through PA levels

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;


void setRadioParameters() {
    switch (dataRateIndex) {
        case 0: radioA.setDataRate(RF24_250KBPS); radioB.setDataRate(RF24_250KBPS); radioC.setDataRate(RF24_250KBPS); break;
        case 1: radioA.setDataRate(RF24_1MBPS);  radioB.setDataRate(RF24_1MBPS); radioC.setDataRate(RF24_1MBPS); break;
        case 2: radioA.setDataRate(RF24_2MBPS); radioB.setDataRate(RF24_2MBPS); radioC.setDataRate(RF24_2MBPS);break;
    }
    
    switch (paLevelIndex) {
        case 0: radioA.setPALevel(RF24_PA_MIN); radioB.setPALevel(RF24_PA_MIN); radioC.setPALevel(RF24_PA_MIN); break;
        case 1: radioA.setPALevel(RF24_PA_LOW); radioB.setPALevel(RF24_PA_LOW); radioC.setPALevel(RF24_PA_LOW); break;
        case 2: radioA.setPALevel(RF24_PA_HIGH); radioB.setPALevel(RF24_PA_HIGH); radioC.setPALevel(RF24_PA_HIGH); break;
        case 3: radioA.setPALevel(RF24_PA_MAX); radioB.setPALevel(RF24_PA_MAX); radioC.setPALevel(RF24_PA_MAX); break;
    }

    Serial.print("Data Rate: ");
    Serial.println(dataRateIndex);
    Serial.print("PA Level: ");
    Serial.println(paLevelIndex);
}

void radioSetChannel(int channels) {
    radioA.setChannel(channels);
    radioB.setChannel(channels);
    radioC.setChannel(channels);
}

void jammer() {
  int methode = 1;
  
  const char text[] = { 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55,
                        0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55 };

  if (methode = 0) {                    
    for (int i = ((channels * 5) + 1); i < ((channels * 5) + 23); i++) {
        radioSetChannel(i);
        bool resultA = radioA.write(&text, sizeof(text));
        bool resultB = radioB.write(&text, sizeof(text));
        bool resultC = radioC.write(&text, sizeof(text));
        
        delay(10);
    }
  }
  
  if (methode = 1) {
    for (int i = 0; i < 22; i++) { // Jam across 22 channels
    int channelA = ((channels * 5) + 1) + i;
    int channelB = ((channels * 5) + 1) + i + 1;
    int channelC = ((channels * 5) + 1) + i + 2;

    // Set each radio to a different channel
    radioA.setChannel(channelA);
    radioB.setChannel(channelB);
    radioC.setChannel(channelC);

    // Transmit payload on all three channels simultaneously
    radioA.write(&text, sizeof(text));
    radioB.write(&text, sizeof(text));
    radioC.write(&text, sizeof(text));

    delay(10); // Delay before hopping to the next set of channels
    }
  }
}

void pressBt01() {
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();
    if (interrupt_time - last_interrupt_time > 200) {
        if (channels < 14) {
            channels++;
        } else {
            channels = 1;
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

void configure(RF24 &radio) {
  radio.begin();
  radio.openWritingPipe(0xFFFFFFFFFF);
  radio.setAutoAck(false);
  radio.stopListening();
  radio.setRetries(0, 0);
  radio.setPALevel(RF24_PA_MAX, true);
  radio.setDataRate(RF24_2MBPS);
  radio.setCRCLength(RF24_CRC_DISABLED);
  radio.printPrettyDetails();
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
    
    pinMode(CE_A, OUTPUT);
    pinMode(CSN_A, OUTPUT);
    
    pinMode(CE_B, OUTPUT);
    pinMode(CSN_B, OUTPUT);
    
    pinMode(CE_C, OUTPUT);
    pinMode(CSN_C, OUTPUT);

    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.sendBuffer();


    configure(radioA);
    configure(radioB);
    configure(radioC);

    //radio.begin();
    setRadioParameters();
    //radio.openWritingPipe(address);
    //radio.stopListening();

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
        Serial.println("Starting jamming on channel " + String(channels));
        jammer();
    }  
}
