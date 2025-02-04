#include "WiFiManager.h"

WiFiManager::WiFiManager(void(*onWiFiEvent)(WiFiEvent_t)) {

  WiFi.mode(WIFI_STA);
  WiFi.onEvent(onWiFiEvent);

}

String WiFiManager::listNetworks() {

  //WiFi.setAutoReconnect(false);

  byte numSsid;

  DEBUG_PRINTLN("\nScanning WiFi networks");

  //WiFi.mode(WIFI_STA);
  //WiFi.disconnect();
  //delay(100);

  while(!(numSsid = WiFi.scanNetworks())) continue;

  // print the list of networks seen:

  DEBUG_PRINTF("\nNumber of available networks: %d", numSsid);

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

  //WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  status = WiFi.waitForConnectResult();

  return status;

}

uint8_t WiFiManager::connectToWiFi(void) {

  uint8_t status;

  WiFi.begin();

  status = WiFi.waitForConnectResult();

  return status;

}

bool WiFiManager::eraseConfiguration(void) {

  bool status = WiFi.eraseAP();
  
  esp_wifi_start();

  return status;

}

void WiFiManager::disconnect(bool wiFiOff, bool eraseAp) {

  WiFi.disconnect(wiFiOff, eraseAp);

  DEBUG_PRINTLN("Disconnected from WiFi network");
  
}