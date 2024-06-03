#include <utility>
#include <Arduino.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <nvs_flash.h>

#include "Configuration.h"

std::pair<ConnectionSettings, bool> Configuration::getConnectionSettings() {

  ConnectionSettings settings;
  bool hasPreferences;

  preferences.begin("preferences", false);

  settings.serverCert = preferences.getString("serverCert");
  settings.clientCert = preferences.getString("clientCert");
  settings.privateKey = preferences.getString("privateKey");
  settings.mqttEndpoint = preferences.getString("mqttEndpoint");

  preferences.end();

  if (hasPreferences = (settings.serverCert != nullptr && settings.clientCert != nullptr && settings.privateKey != nullptr && settings.mqttEndpoint != nullptr)) {

    Serial.println("\Configuration settings found on this device");
    
  } else {

    Serial.println("\nNo Configuration settings found on this device");

  }

  return std::make_pair(settings, hasPreferences);

}

void Configuration::setConnectionSettings(ConnectionSettings settings) {
  
  preferences.begin("preferences", false);

  preferences.putString("serverCert", settings.serverCert);
  preferences.putString("clientCert", settings.clientCert);
  preferences.putString("privateKey", settings.privateKey);
  preferences.putString("mqttEndpoint", settings.mqttEndpoint);

  preferences.end();

  Serial.printf("\nConfiguration set: server ca certificate: %s; client certificate: %s; client private key: %s; mqtt endpoint: %s", settings.serverCert.c_str(), settings.clientCert.c_str(), settings.privateKey.c_str(), settings.mqttEndpoint.c_str());

}

void Configuration::deleteConnectionSettings(void) {

  preferences.begin("preferences", false);

  preferences.clear();
  Serial.println("Configuration deleted");

  preferences.end();

}

void Configuration::reset(void) {

  nvs_flash_erase();
  nvs_flash_init();

}






