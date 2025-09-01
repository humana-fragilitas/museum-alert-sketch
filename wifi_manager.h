#ifndef WIFI_MANAGER
#define WIFI_MANAGER

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>

#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <vector>
#include <algorithm>

#include "config.h"

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
      static TaskHandle_t wifiMonitorTaskHandle;
      static bool monitoringEnabled;
      static bool wasConnected;
      static unsigned long lastConnectivityTest;
      static void wifiMonitorTaskWrapper(void* pvParameters) noexcept;
      static void wifiMonitorTask();
      static bool testConnectivity() noexcept;
      static void handleConnectionStateChange(bool connected) noexcept;

   public:
      static void initialize() noexcept;
      static void listNetworks(JsonArray& arr);
      static uint8_t connectToWiFi(const char *ssid, const char *pass) noexcept;
      static uint8_t connectToWiFi() noexcept;
      static bool eraseConfiguration() noexcept;
      static bool disconnect(bool wiFiOff = false, bool eraseAp = false) noexcept;
      static void onWiFiEvent(WiFiEvent_t event) noexcept;
      static bool isConnected() noexcept;
      static void reset() noexcept;
      static void startMonitoring() noexcept;
      static void stopMonitoring();
      static bool isMonitoringActive() noexcept;

};

#endif