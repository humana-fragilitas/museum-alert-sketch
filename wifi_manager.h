#ifndef WIFI_MANAGER
#define WIFI_MANAGER

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <esp_wifi.h>
#include <vector>
#include <algorithm>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "macros.h"
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
    
    static void wifiMonitorTaskWrapper(void* pvParameters);
    static void wifiMonitorTask();
    static bool testConnectivity();
    static void handleConnectionStateChange(bool connected);

public:
    static void initialize();
    static void listNetworks(JsonArray& arr);
    static uint8_t connectToWiFi(const char *ssid, const char *pass);
    static uint8_t connectToWiFi();
    static bool eraseConfiguration();
    static bool disconnect(bool wiFiOff = false, bool eraseAp = false);
    static void onWiFiEvent(WiFiEvent_t event);
    static bool isConnected();
    static void reset();
    
    // New monitoring methods
    static void startMonitoring();
    static void stopMonitoring();
    static bool isMonitoringActive();
};

#endif