#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <esp_wifi.h>

#include "Pins.h"
#include "Macros.h"

struct WiFiNetwork {
  String ssid;
  int32_t rssi;
  wifi_auth_mode_t encryptionType;
};

class WiFiManager {

  private:
    static WiFiEvent_t lastEvent;
    static void onWiFiEvent(WiFiEvent_t event);

  public:
    static void initialize();
    static bool isConnected();
    static String listNetworks();
    static uint8_t connectToWiFi(String ssid, String pass);
    static uint8_t connectToWiFi(void);
    static bool eraseConfiguration(void);
    static void disconnect(bool wiFiOff = false, bool eraseAp = false);

};