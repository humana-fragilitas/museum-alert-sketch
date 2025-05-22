#include "wifi_manager.h"

void WiFiManager::initialize() {
  WiFi.mode(WIFI_STA);
  WiFi.onEvent(WiFiManager::onWiFiEvent);
}

#include <ArduinoJson.h>
#include <vector>
#include <algorithm>

void WiFiManager::listNetworks(JsonArray& arr) {
  
  byte numSsid = WiFi.scanNetworks();
  DEBUG_PRINTF("Number of available WiFi networks: %d\n", numSsid);

  // Define a struct to hold WiFi info
  struct WiFiEntry {
    String ssid;
    int rssi;
    int encryptionType;
  };

  std::vector<WiFiEntry> networks;

  for (int i = 0; i < numSsid; ++i) {
    networks.push_back({ WiFi.SSID(i), WiFi.RSSI(i), WiFi.encryptionType(i) });
  }

  WiFi.scanDelete();

  std::sort(networks.begin(), networks.end(), [](const WiFiEntry &a, const WiFiEntry &b) {
    return a.rssi > b.rssi;
  });

  //int maxResults = std::min(10, static_cast<int>(networks.size()));

  // JsonDocument doc;
  //JsonArray arr = doc.to<JsonArray>();

  for (int i = 0; i < networks.size(); ++i) {
    JsonObject wiFiEntry = arr.add<JsonObject>();
    wiFiEntry["ssid"] = networks[i].ssid;
    wiFiEntry["rssi"] = networks[i].rssi;
    wiFiEntry["encryptionType"] = networks[i].encryptionType;

    DEBUG_PRINTF("%d) %s | signal: %d dBm | encryption: %d\n",
                  i + 1, networks[i].ssid.c_str(), networks[i].rssi, networks[i].encryptionType);
  }

  String jsonString;
  serializeJson(arr, jsonString);

  DEBUG_PRINTLN(jsonString.c_str());

  //return arr;

}

uint8_t WiFiManager::connectToWiFi(const char *ssid, const char *pass) {

  if (!ssid || ssid[0] == '\0' || !pass) {
      DEBUG_PRINTLN("Invalid WiFi credentials");
      return WL_CONNECT_FAILED;
  }

  DEBUG_PRINTF("Connecting to WiFi SSID: %s\n", ssid);
  DEBUG_PRINTF("Connecting to WiFi password: %s\n", pass);

  WiFi.disconnect(true);
  // delay(250); TO DO: is it necessary?
  WiFi.begin(ssid, pass);
  return WiFi.waitForConnectResult(10000);

}

uint8_t WiFiManager::connectToWiFi() {

  DEBUG_PRINTF("Trying to connect to a previously set WiFi endpoint; waiting for %d seconds...\n", (Timing::WIFI_AUTO_CONNECTION_TIMEOUT_MS / 1000));
  WiFi.begin();
  return WiFi.waitForConnectResult(Timing::WIFI_AUTO_CONNECTION_TIMEOUT_MS);

}

bool WiFiManager::eraseConfiguration() {

  bool success = WiFi.eraseAP();
  esp_wifi_start();

  DEBUG_PRINTLN(success ? "Erased WiFi configuration" : "Failed to erase WiFi configuration");

  return success;

}

bool WiFiManager::disconnect(bool wiFiOff, bool eraseAp) {

  bool success = WiFi.disconnect(wiFiOff, eraseAp);

  DEBUG_PRINTLN(success ? "Disconnected from WiFi network" : "Failed to disconnect from WiFi network");

  return success;

}

void WiFiManager::onWiFiEvent(WiFiEvent_t event) {

  switch (event) {

    case ARDUINO_EVENT_WIFI_READY:
      DEBUG_PRINTLN("WiFi interface ready");
      break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
      DEBUG_PRINTLN("WiFi scan complete");
      break;
    case ARDUINO_EVENT_WIFI_STA_START:
      DEBUG_PRINTLN("WiFi client started");
      break;
    case ARDUINO_EVENT_WIFI_STA_STOP:
      DEBUG_PRINTLN("WiFi client stopped");
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      DEBUG_PRINTLN("Connected to WiFi access point");
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      DEBUG_PRINTLN("Disconnected from WiFi access point");
      break;
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
      DEBUG_PRINTLN("Authentication mode changed");
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      DEBUG_PRINTF("WiFi IP Address: %s\n", WiFi.localIP().toString().c_str());
      break;
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
      DEBUG_PRINTLN("Lost IP address");
      break;

  }

}

bool WiFiManager::isConnected() {
  return WiFi.isConnected();
}

// possible fix to alternate wifi and ble
void WiFiManager::reset() {

  esp_wifi_stop();  // Stop WiFi driver
  esp_wifi_deinit(); // Deinitialize WiFi
  delay(100);       // Short delay
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);  // Reinitialize WiFi
  esp_wifi_start(); // Start WiFi again

}