/* ____________________________
   This software is licensed under the MIT License:
   https://github.com/cifertech/nrfbox
   ________________________________________ */

#include <Arduino.h>
#include <U8g2lib.h>
#include <stdint.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#include "icon.h"
#include "neopixel.h"
#include "setting.h"

#include "scanner.h"
#include "analyzer.h"
#include "jammer.h"
#include "blejammer.h"
#include "spoofer.h"
#include "sourapple.h"
#include "blescan.h"
#include "wifiscan.h"
#include "blackout.h"


#define CE_PIN_A  5
#define CSN_PIN_A 17

#define CE_PIN_B  16
#define CSN_PIN_B 4

#define CE_PIN_C  15
#define CSN_PIN_C 2

RF24 RadioA(CE_PIN_A, CSN_PIN_A);
RF24 RadioB(CE_PIN_B, CSN_PIN_B);
RF24 RadioC(CE_PIN_C, CSN_PIN_C);

//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0); // [full framebuffer, size = 1024 bytes]
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

Adafruit_NeoPixel pixels(1, 14, NEO_GRB + NEO_KHZ800);

extern uint8_t oledBrightness;


const unsigned char* bitmap_icons[11] = {
  bitmap_icon_scanner,
  bitmap_icon_analyzer,
  bitmap_icon_jammer,
  bitmap_icon_kill,
  bitmap_icon_ble_jammer,
  bitmap_icon_spoofer,
  bitmap_icon_apple,
  bitmap_icon_ble,
  bitmap_icon_wifi,
  bitmap_icon_about,
  bitmap_icon_setting
};


const int NUM_ITEMS = 11; 
const int MAX_ITEM_LENGTH = 20; 

char menu_items [NUM_ITEMS] [MAX_ITEM_LENGTH] = {  
  { "Scanner" }, 
  { "Analyzer" },
  { "WLAN Jammer" },
  { "Proto Kill" },
  { "BLE Jammer" },
  { "BLE Spoofer" }, 
  { "Sour Apple" },
  { "BLE Scan" },
  { "WiFi Scan" },
  { "About" },
  { "Setting" }
 };
 

#define BUTTON_UP_PIN 26 
#define BUTTON_SELECT_PIN 32
#define BUTTON_DOWN_PIN 33 


int button_up_clicked = 0; 
int button_select_clicked = 0; 
int button_down_clicked = 0; 

int item_selected = 0; 

int item_sel_previous; 
int item_sel_next; 

int current_screen = 0;  


void about() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(7, 15, "CiferTech@gmail.com");
  u8g2.drawStr(12, 35, "GitHub/cifertech");
  u8g2.drawStr(7, 55, "instagram/cifertech");
  u8g2.sendBuffer();
}

void configureNrf(RF24 &radio) {
  radio.begin();
  radio.setAutoAck(false);
  radio.stopListening();
  radio.setRetries(0, 0);
  radio.setPALevel(RF24_PA_MAX, true);
  radio.setDataRate(RF24_2MBPS);
  radio.setCRCLength(RF24_CRC_DISABLED);
}


void setup() {

  neopixelSetup();

  configureNrf(RadioA);
  configureNrf(RadioB);
  configureNrf(RadioC);

  EEPROM.begin(512); 
  oledBrightness = EEPROM.read(1);
  
  u8g2.begin();
  u8g2.setContrast(oledBrightness);
  u8g2.setBitmapMode(1);

  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_ncenB14_tr); 
  int16_t nameWidth = u8g2.getUTF8Width("nRF-BOX"); 
  int16_t nameX = (128 - nameWidth) / 2;            
  u8g2.setCursor(nameX, 25);                      
  u8g2.print("nRF-BOX");

  u8g2.setFont(u8g2_font_ncenB08_tr); 
  int16_t creditWidth = u8g2.getUTF8Width("by CiferTech");
  int16_t creditX = (106 - creditWidth) / 2;
  u8g2.setCursor(creditX, 40);
  u8g2.print("by CiferTech");

  u8g2.setFont(u8g2_font_6x10_tf); 
  int16_t versionWidth = u8g2.getUTF8Width("v2.5.0");
  int16_t versionX = (128 - versionWidth) / 2;
  u8g2.setCursor(versionX, 60);
  u8g2.print("v2.5.0");
  
  u8g2.sendBuffer(); 
  delay(3000);

  u8g2.clearBuffer();

  u8g2.drawXBMP(0, 0, 128, 64, logo_cifer);

  u8g2.sendBuffer(); 
  delay(250);   

  pinMode(BUTTON_UP_PIN, INPUT_PULLUP); 
  pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP); 
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP); 

}


void loop() {

  if (current_screen == 0) { // MENU SCREEN
    
      if ((digitalRead(BUTTON_UP_PIN) == LOW) && (button_up_clicked == 0)) { 
        item_selected = item_selected - 1; 
        button_up_clicked = 1; 
        if (item_selected < 0) { 
          item_selected = NUM_ITEMS-1;
        }
      }
      else if ((digitalRead(BUTTON_DOWN_PIN) == LOW) && (button_down_clicked == 0)) { 
        item_selected = item_selected + 1; 
        button_down_clicked = 1; 
        if (item_selected >= NUM_ITEMS) { 
          item_selected = 0;
          }
      } 

      if ((digitalRead(BUTTON_UP_PIN) == HIGH) && (button_up_clicked == 1)) { 
        button_up_clicked = 0;
      }
      if ((digitalRead(BUTTON_DOWN_PIN) == HIGH) && (button_down_clicked == 1)) { 
        button_down_clicked = 0;
      }
  }


  bool callAbout = true;

  if ((digitalRead(BUTTON_SELECT_PIN) == LOW) && (button_select_clicked == 0)) { 
     button_select_clicked = 1; 


if (current_screen == 0 && item_selected == 10) {
  settingSetup();
    while (item_selected == 10) {
        if (digitalRead(BUTTON_SELECT_PIN) == HIGH) {
            if (callAbout) {
                settingLoop();
                callAbout = false;  // Toggle the state to not call about()
            } else {
                break;  // Toggle the state to break the loop
                callAbout = true;  // Reset the state for the next cycle
            }

            while (digitalRead(BUTTON_SELECT_PIN) == HIGH) {
                // Wait for the button to be released
                if (callAbout = true){
                  break;
                }
            }
        }
    }
  }     


if (current_screen == 0 && item_selected == 9) {
    while (item_selected == 9) {
        if (digitalRead(BUTTON_SELECT_PIN) == HIGH) {
            if (callAbout) {
                about();
                callAbout = false;  // Toggle the state to not call about()
            } else {
                break;  // Toggle the state to break the loop
                callAbout = true;  // Reset the state for the next cycle
            }

            while (digitalRead(BUTTON_SELECT_PIN) == HIGH) {
                // Wait for the button to be released
                if (callAbout = true){
                  break;
                }
            }
        }
    }
  }


if (current_screen == 0 && item_selected == 8) {
  wifiscanSetup();
    while (item_selected == 8) {
        if (digitalRead(BUTTON_SELECT_PIN) == HIGH) { 
         wifiscanLoop();     
            if (callAbout) {                             
                callAbout = false;  // Toggle the state to not call about()
            } else {
                break;  // Toggle the state to break the loop
                callAbout = true;  // Reset the state for the next cycle
            }

            while (digitalRead(BUTTON_SELECT_PIN) == HIGH) {
                // Wait for the button to be released
                
                if (callAbout = true){
                  break;
                }
            }
        }
    }
}


if (current_screen == 0 && item_selected == 7) {
  blescanSetup();
    while (item_selected == 7) {
        if (digitalRead(BUTTON_SELECT_PIN) == HIGH) { 
          blescanLoop();     
            if (callAbout) {                
                callAbout = false;  // Toggle the state to not call about()
            } else {
                break;  // Toggle the state to break the loop
                callAbout = true;  // Reset the state for the next cycle
            }

            while (digitalRead(BUTTON_SELECT_PIN) == HIGH) {
                // Wait for the button to be released
                
                if (callAbout = true){
                  break;
                }
            }
        }
    }
}


if (current_screen == 0 && item_selected == 6) {
  sourappleSetup();
    while (item_selected == 6) {
        if (digitalRead(BUTTON_SELECT_PIN) == HIGH) { 
          sourappleLoop();     
            if (callAbout) {                
                callAbout = false;  // Toggle the state to not call about()
            } else {
                break;  // Toggle the state to break the loop
                callAbout = true;  // Reset the state for the next cycle
            }

            while (digitalRead(BUTTON_SELECT_PIN) == HIGH) {
                // Wait for the button to be released
                
                if (callAbout = true){
                  break;
                }
            }
        }
    }
}     


if (current_screen == 0 && item_selected == 5) {
  spooferSetup();
    while (item_selected == 5) {
        if (digitalRead(BUTTON_SELECT_PIN) == HIGH) { 
          spooferLoop();     
            if (callAbout) {                
                callAbout = false;  // Toggle the state to not call about()
            } else {
                break;  // Toggle the state to break the loop
                callAbout = true;  // Reset the state for the next cycle
            }

            while (digitalRead(BUTTON_SELECT_PIN) == HIGH) {
                // Wait for the button to be released
                
                if (callAbout = true){
                  break;
                }
            }
        }
    }
}
     

if (current_screen == 0 && item_selected == 4) {
  blejammerSetup();
    while (item_selected == 4) {
        if (digitalRead(BUTTON_SELECT_PIN) == HIGH) { 
          blejammerLoop();     
            if (callAbout) {                
                callAbout = false;  // Toggle the state to not call about()
            } else {
                break;  // Toggle the state to break the loop
                callAbout = true;  // Reset the state for the next cycle
            }

            while (digitalRead(BUTTON_SELECT_PIN) == HIGH) {
                // Wait for the button to be released
                
                if (callAbout = true){
                  break;
                }
            }
        }
    }
}


if (current_screen == 0 && item_selected == 3) {
  blackoutSetup();
    while (item_selected == 3) {
        if (digitalRead(BUTTON_SELECT_PIN) == HIGH) { 
          blackoutLoop();     
            if (callAbout) {                
                callAbout = false;  // Toggle the state to not call about()
            } else {
                break;  // Toggle the state to break the loop
                callAbout = true;  // Reset the state for the next cycle
            }

            while (digitalRead(BUTTON_SELECT_PIN) == HIGH) {
                // Wait for the button to be released
                
                if (callAbout = true){
                  break;
                }
            }
        }
    }
}
     

if (current_screen == 0 && item_selected == 2) {
  jammerSetup();
    while (item_selected == 2) {
        if (digitalRead(BUTTON_SELECT_PIN) == HIGH) { 
          jammerLoop();     
            if (callAbout) {                
                callAbout = false;  // Toggle the state to not call about()
            } else {
                break;  // Toggle the state to break the loop
                callAbout = true;  // Reset the state for the next cycle
            }

            while (digitalRead(BUTTON_SELECT_PIN) == HIGH) {
                // Wait for the button to be released
                
                if (callAbout = true){
                  break;
                }
            }
        }
    }
}     
     
 
if (current_screen == 0 && item_selected == 1) {
  analyzerSetup();
    while (item_selected == 1) {
        if (digitalRead(BUTTON_SELECT_PIN) == HIGH) { 
          analyzerLoop();     
            if (callAbout) {                
                callAbout = false;  // Toggle the state to not call about()
            } else {
                break;  // Toggle the state to break the loop
                callAbout = true;  // Reset the state for the next cycle
            }

            while (digitalRead(BUTTON_SELECT_PIN) == HIGH) {
                // Wait for the button to be released
                
                if (callAbout = true){
                  break;
                }
            }
        }
    }
}    
   

if (current_screen == 0 && item_selected == 0) {
  scannerSetup();
    while (item_selected == 0) {
        if (digitalRead(BUTTON_SELECT_PIN) == HIGH) {       
            if (callAbout) {
                scannerLoop();   
                callAbout = false;  // Toggle the state to not call about()
            } else {
                break;  // Toggle the state to break the loop
                callAbout = true;  // Reset the state for the next cycle
            }

            while (digitalRead(BUTTON_SELECT_PIN) == HIGH) {
                // Wait for the button to be released
                if (callAbout = true){
                  break;
                }
            }
        }
    }
 }  

}  

  if ((digitalRead(BUTTON_SELECT_PIN) == HIGH) && (button_select_clicked == 1)) { 
    button_select_clicked = 0;
  }


  item_sel_previous = item_selected - 1;
  if (item_sel_previous < 0) {item_sel_previous = NUM_ITEMS - 1;} 
  item_sel_next = item_selected + 1;  
  if (item_sel_next >= NUM_ITEMS) {item_sel_next = 0;} 



  u8g2.clearBuffer();  

    if (current_screen == 0) {
     
      u8g2.drawXBMP(0, 22, 128, 21, bitmap_item_sel_outline);

      u8g2.setFont(u8g_font_7x14);
      u8g2.drawStr(25, 15, menu_items[item_sel_previous]); 
      u8g2.drawXBMP( 4, 2, 16, 16, bitmap_icons[item_sel_previous]);          

      u8g2.setFont(u8g_font_7x14B);    
      u8g2.drawStr(25, 15+20+2, menu_items[item_selected]);   
      u8g2.drawXBMP( 4, 24, 16, 16, bitmap_icons[item_selected]);     

      u8g2.setFont(u8g_font_7x14);     
      u8g2.drawStr(25, 15+20+20+2+2, menu_items[item_sel_next]);   
      u8g2.drawXBMP( 4, 46, 16, 16, bitmap_icons[item_sel_next]);  

      u8g2.drawXBMP(128-8, 0, 8, 64, bitmap_scrollbar_background);

      u8g2.drawBox(125, 64/NUM_ITEMS * item_selected, 3, 64/NUM_ITEMS);             
    } 
    
  u8g2.sendBuffer(); 

}
