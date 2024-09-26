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

  //settings.serverCert = preferences.getString("serverCert");
  settings.clientCert = preferences.getString("clientCert");
  settings.privateKey = preferences.getString("privateKey");
  settings.mqttEndpoint = preferences.getString("mqttEndpoint");

  preferences.end();

  // Note: assignment
  if (hasPreferences = (!settings.clientCert.isEmpty() &&
                        !settings.privateKey.isEmpty() &&
                        !settings.mqttEndpoint.isEmpty())) {

    Serial.println("\Configuration settings found on this device");
    
  } else {

    Serial.println("\nNo Configuration settings found on this device");

  }

  return std::make_pair(settings, hasPreferences);

}

void Configuration::setConnectionSettings(ConnectionSettings settings) {
  
  preferences.begin("preferences", false);

  //preferences.putString("serverCert", settings.serverCert);
  preferences.putString("clientCert", settings.clientCert);
  preferences.putString("privateKey", settings.privateKey);
  preferences.putString("mqttEndpoint", settings.mqttEndpoint);

  preferences.end();

  Serial.printf("\nConfiguration set: client certificate: %s;\nclient private key: %s;\n mqtt endpoint: %s",
    settings.clientCert.c_str(), settings.privateKey.c_str(), settings.mqttEndpoint.c_str());

}

void Configuration::deleteConnectionSettings(void) {

  preferences.begin("preferences", false);

  preferences.clear();
  Serial.println("Configuration deleted");

  preferences.end();

}

std::pair<ConnectionSettings, bool> Configuration::getProvisioningConnectionSettings() {

  bool hasProvisioningSettings = (!provisioningSettings.clientCert.isEmpty() &&
                                  !provisioningSettings.privateKey.isEmpty() &&
                                  !provisioningSettings.mqttEndpoint.isEmpty());

  return std::make_pair(provisioningSettings, hasProvisioningSettings);

}

void Configuration::setProvisioningConnectionSettings(ConnectionSettings settings) {
  
  provisioningSettings.clientCert = settings.clientCert;
  provisioningSettings.privateKey = settings.privateKey;
  provisioningSettings.mqttEndpoint = settings.mqttEndpoint;

  Serial.printf("\nProvisioning configuration set: client certificate: %s; client private key: %s; mqtt endpoint: %s",
    provisioningSettings.clientCert.c_str(),
    provisioningSettings.privateKey.c_str(),
    provisioningSettings.mqttEndpoint.c_str());

}

void Configuration::deleteProvisioningConnectionSettings(void) {

  provisioningSettings.clientCert.clear();
  provisioningSettings.privateKey.clear();
  provisioningSettings.mqttEndpoint.clear();

}

void Configuration::reset(void) {

  nvs_flash_erase();
  nvs_flash_init();

  this->deleteProvisioningConnectionSettings();

}






