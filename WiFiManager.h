#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <esp_wifi.h>

#include "Pins.h"
#include "Macros.h"

#define SSID_MAX_LEN  32    // Max length of SSID (WiFi name)
#define PASS_MAX_LEN  64    // Max length of WiFi password
#define JSON_SIZE     1024  // Buffer size for JSON serialization

struct WiFiNetwork {
    char ssid[SSID_MAX_LEN];
    int32_t rssi;
    wifi_auth_mode_t encryptionType;
};

class WiFiManager {
  private:
    static void onWiFiEvent(WiFiEvent_t event);

  public:
    static void initialize();
    static bool isConnected();
    void listNetworks(char *jsonBuffer, size_t bufferSize);
    static uint8_t connectToWiFi(const char *ssid, const char *pass);
    static uint8_t connectToWiFi();
    static bool eraseConfiguration();
    static void disconnect(bool wiFiOff = false, bool eraseAp = false);
};