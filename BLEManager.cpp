#include <ArduinoBLE.h>
#include <ArduinoJson.h>

#include "BLEManager.h"
#include "Sensor.h"

const char* BLEManager::deviceServiceUuid = "19b10000-e8f2-537e-4f6c-d104768a1214";
const char* BLEManager::deviceServiceConfigurationCharacteristicUuid = "19b10001-e8f2-537e-4f6c-d104768a1214";
const char* BLEManager::deviceServiceSsidsCharacteristicUuid = "19b10001-e8f2-537e-4f6c-d104768a1213";
BLEService BLEManager::configurationService(deviceServiceUuid);
BLEStringCharacteristic BLEManager::wiFiSsidsCharacteristic(deviceServiceSsidsCharacteristicUuid, BLERead, 4096);
BLEStringCharacteristic BLEManager::configurationCharacteristic(deviceServiceConfigurationCharacteristicUuid, BLERead | BLEWrite, 512);

BLEManager::BLEManager() {}

bool BLEManager::initializeBLEConfigurationService() {

  const char *sensorName = Sensor::sensorName.c_str();

  if (!BLE.begin()) {
    Serial.println("\nStarting Bluetooth® Low Energy module failed!");
    return false;
  }

  Serial.println("Bluetooth® Low Energy module initialized");

  BLE.setDeviceName(sensorName);
  BLE.setLocalName(sensorName); 
  BLE.setAdvertisedService(configurationService);
  configurationService.addCharacteristic(configurationCharacteristic);
  configurationService.addCharacteristic(wiFiSsidsCharacteristic);
  BLE.addService(configurationService);
  configurationCharacteristic.writeValue("");
  wiFiSsidsCharacteristic.writeValue("");
  BLE.advertise();

  Serial.printf("Sensor Bluetooth® Low Energy module advertising itself with local name %s", sensorName);

  return true;

}

void BLEManager::configureViaBLE() {

  BLEDevice central = BLE.central();
  Serial.println("\nDiscovering central device...");

  if (central) {

    Serial.println("\nConnected to central device!");
    Serial.println("\nDevice MAC address: ");
    Serial.println(central.address());

    while (central.connected()) {
      if (configurationCharacteristic.written()) {
        _onWiFiCredentials(configurationCharacteristic.value());
      }
    }
    
    Serial.println("\nDisconnected to central device!");

  }

}

/* std::pair<WiFiCredentials, ConnectionSettings>*/ Settings BLEManager::configureWiFi(String json) {

  BLEDevice central = BLE.central();
  Serial.println("\nDiscovering central device...");
  //delay(500);

  ConnectionSettings connectionSettings;
  WiFiCredentials wiFiCredentials;
  Settings settings;

  if (central) {

    Serial.println("\nConnected to central device!");
    Serial.println("\nDevice MAC address: ");
    Serial.println(central.address());

    while (central.connected()) {

      wiFiSsidsCharacteristic.writeValue(json);

      if (configurationCharacteristic.written()) {

        String configuration = configurationCharacteristic.value();
        Serial.printf("\nReceived configuration: %s", configuration.c_str());

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, configuration);

        if (error) {
          Serial.println("Failed to deserialize WiFi configuration json: ");
          Serial.println(error.c_str());
          continue;
        }

        wiFiCredentials.ssid = doc["ssid"].as<String>();
        wiFiCredentials.password = doc["pass"].as<String>();

        connectionSettings.clientCert = doc["tempCertPem"].as<String>();
        connectionSettings.privateKey = doc["tempPrivateKey"].as<String>();
        connectionSettings.mqttEndpoint = doc["mqttEndpoint"].as<String>();

        settings.connectionSettings = connectionSettings;
        settings.wiFiCredentials = wiFiCredentials;
        
        break;

      }

    }
    
    Serial.println("\nDisconnected from central device!");

    return settings; //std::make_pair(credentials, settings);
    
  }

}

void BLEManager::disconnect() {

  BLEDevice central = BLE.central();
  central.disconnect();

}