#include "wifi_manager.h"

void WiFiManager::initialize() {
  WiFi.mode(WIFI_STA);
  WiFi.onEvent(WiFiManager::onWiFiEvent);
}

void WiFiManager::listNetworks(char *jsonBuffer) {

  // temporary fix
  WiFi.mode(WIFI_STA);

  byte numSsid = WiFi.scanNetworks();
  DEBUG_PRINTF("Number of available WiFi networks: %d\n", numSsid);

  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();

  for (int i = 0; i < numSsid; ++i) {
    JsonObject wiFiEntry = arr.add<JsonObject>();
    wiFiEntry["ssid"] = WiFi.SSID(i);
    wiFiEntry["rssi"] = WiFi.RSSI(i);
    wiFiEntry["encryptionType"] = WiFi.encryptionType(i);

    DEBUG_PRINTF("%u) %s | signal: %d dBm | encryption: %d\n",
                  i, WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.encryptionType(i));
  }

  WiFi.scanDelete();

  // Serialize JSON safely within buffer size
  serializeJson(arr, jsonBuffer, sizeof(jsonBuffer));

  // temporary fix
  WiFi.mode(WIFI_OFF);
  // add: WiFi.disconnect(true); ?

  //Source: https://forum.arduino.cc/t/how-to-broadcast-data-from-one-iot-to-another-iot-or-ble-sense-via-bluetooth/675794/7
  
}

uint8_t WiFiManager::connectToWiFi(const char *ssid, const char *pass) {

  if (!ssid || ssid[0] == '\0' || !pass) {
      DEBUG_PRINTLN("Invalid WiFi credentials");
      return WL_CONNECT_FAILED;
  }

  DEBUG_PRINTF("Connecting to WiFi SSID: %s\n", ssid);

  WiFi.begin(ssid, pass);
  return WiFi.waitForConnectResult();

}

uint8_t WiFiManager::connectToWiFi() {

  DEBUG_PRINTLN("Trying to connect to a previously set WiFi endpoint...");
  WiFi.begin();
  return WiFi.waitForConnectResult();

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