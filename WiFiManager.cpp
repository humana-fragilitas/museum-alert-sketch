#include "WiFiManager.h"

void WiFiManager::initialize() {

  WiFi.mode(WIFI_STA);
  WiFi.onEvent(WiFiManager::onWiFiEvent);

}

String WiFiManager::listNetworks() {

  //WiFi.setAutoReconnect(false);

  byte numSsid;

  DEBUG_PRINTLN("Scanning WiFi networks");

  //WiFi.mode(WIFI_STA);
  //WiFi.disconnect();
  //delay(100);

  while(!(numSsid = WiFi.scanNetworks())) continue;

  // print the list of networks seen:

  DEBUG_PRINTF("Number of available WiFi networks: %d\n", numSsid);

  // print the network number and name for each network found:

  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();

  for (int i = 0; i < numSsid; ++i) {

    DEBUG_PRINTF("%u) %s | signal: %d dbm | encryption: %d\n",
      i, WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.encryptionType(i));

    JsonObject wiFiEntry = arr.add<JsonObject>();
    wiFiEntry["ssid"] = WiFi.SSID(i);
    wiFiEntry["rssi"] = WiFi.RSSI(i);
    wiFiEntry["encryptionType"] = WiFi.encryptionType(i);

  }

  WiFi.scanDelete();

  char json[4096];
  serializeJson(arr, json);

  return String(json);

}

uint8_t WiFiManager::connectToWiFi(String ssid, String pass) {

  uint8_t status;

  DEBUG_PRINTLN("Trying to connect to WiFi endpoint...");

  //WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  status = WiFi.waitForConnectResult();

  return status;

}

uint8_t WiFiManager::connectToWiFi(void) {

  uint8_t status;

  DEBUG_PRINTLN("Trying to connect to any previuosly set WiFi endpoint...");

  WiFi.begin();

  status = WiFi.waitForConnectResult();

  return status;

}

bool WiFiManager::eraseConfiguration(void) {

  bool status = WiFi.eraseAP();
  
  esp_wifi_start();

  DEBUG_PRINTLN("Erased WiFi interface configuration");

  return status;

}

void WiFiManager::disconnect(bool wiFiOff, bool eraseAp) {

  WiFi.disconnect(wiFiOff, eraseAp);

  DEBUG_PRINTLN("Disconnected from WiFi network");
  
}

void WiFiManager::onWiFiEvent(WiFiEvent_t event) {

  WiFiManager::lastEvent = event;

  switch (event) {

    case ARDUINO_EVENT_WIFI_READY: 
        DEBUG_PRINTLN("WiFi interface ready");
        break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
        DEBUG_PRINTLN("Completed scan for WiFi access points");
        break;
    case ARDUINO_EVENT_WIFI_STA_START:
        DEBUG_PRINTLN("WiFi client started");
        break;
    case ARDUINO_EVENT_WIFI_STA_STOP:
        DEBUG_PRINTLN("WiFi clients stopped");
        break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        DEBUG_PRINTLN("Connected to WiFi access point");
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        DEBUG_PRINTLN("Disconnected from WiFi access point");
        break;
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        DEBUG_PRINTLN("Authentication mode of access point has changed");
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        DEBUG_PRINTF("WiFi interface obtained IP address: %s\n", String(WiFi.localIP()));
        break;
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        DEBUG_PRINTLN("Lost IP address and IP address is reset to 0");
        break;

  }

}

bool WiFiManager::isConnected() {

  return WiFiManager::lastEvent == ARDUINO_EVENT_WIFI_STA_CONNECTED;
  
}

WiFiEvent_t WiFiManager::lastEvent = ARDUINO_EVENT_WIFI_STA_DISCONNECTED;