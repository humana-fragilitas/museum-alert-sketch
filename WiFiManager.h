#include <Arduino.h>
#include <vector>
#include <ArduinoJson.h>
#include <WiFi.h>

struct WiFiNetwork {
  String ssid;
  int32_t rssi;
  wifi_auth_mode_t encryptionType;
};

class WiFiManager {

  public:
    WiFiManager(void(*onWiFiEvent)(WiFiEvent_t));
    void listNetworks(JsonArray* doc);
    uint8_t connectToWiFi(String ssid, String pass);
    uint8_t connectToWiFi(void);
    bool eraseConfiguration(void);
    void disconnect(bool wiFiOff, bool eraseAp);

};