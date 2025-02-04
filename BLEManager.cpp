#include "BLEManager.h"

const char* BLEManager::deviceServiceUuid = "19b10000-e8f2-537e-4f6c-d104768a1214";
const char* BLEManager::deviceServiceConfigurationCharacteristicUuid = "19b10001-e8f2-537e-4f6c-d104768a1214";
const char* BLEManager::deviceServiceSsidsCharacteristicUuid = "19b10001-e8f2-537e-4f6c-d104768a1213";
BLEService BLEManager::configurationService(deviceServiceUuid);
BLEStringCharacteristic BLEManager::wiFiSsidsCharacteristic(deviceServiceSsidsCharacteristicUuid, BLERead, 4096);
BLEStringCharacteristic BLEManager::configurationCharacteristic(deviceServiceConfigurationCharacteristicUuid, BLERead | BLEWrite, 512);

BLEManager::BLEManager() {}

bool BLEManager::initializeDeviceConfigurationService() {

  const char *sensorName = Sensor::name.c_str();

  if (!BLE.begin()) {
    DEBUG_PRINTLN("Failes to start Bluetooth® Low Energy module! Exiting...");
    return false;
  }

  DEBUG_PRINTLN("Bluetooth® Low Energy module initialized");

  BLE.setDeviceName(sensorName);
  BLE.setLocalName(sensorName); 
  BLE.setAdvertisedService(configurationService);
  configurationService.addCharacteristic(configurationCharacteristic);
  configurationService.addCharacteristic(wiFiSsidsCharacteristic);
  BLE.addService(configurationService);
  configurationCharacteristic.writeValue("");
  wiFiSsidsCharacteristic.writeValue("");
  BLE.advertise();

  DEBUG_PRINTF("Sensor Bluetooth® Low Energy module advertising itself with local name %s\n", sensorName);

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

ProvisioningSettings BLEManager::getDeviceConfiguration(String json) {

  BLEDevice central = BLE.central();
  DEBUG_PRINTLN("Discovering central device via Bluetooth...");
  delay(500);

  ProvisioningSettings provisioningSettings;

  if (central) {

    DEBUG_PRINTF("Connected via Bluetooth to central device with MAC address: %s\n", central.address());
    
    while (central.connected()) {

      wiFiSsidsCharacteristic.writeValue(json);

      if (configurationCharacteristic.written()) {

        String configuration = configurationCharacteristic.value();
        DEBUG_PRINTF("Received configuration via Bluetooth: %s\n", configuration.c_str());

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, configuration);

        if (error) {
          DEBUG_PRINTLN("Failed to deserialize WiFi configuration json:");
          DEBUG_PRINTLN(error.c_str());
          continue;
        }

        provisioningSettings.wiFiCredentials.ssid = doc["ssid"].as<String>();
        provisioningSettings.wiFiCredentials.password = doc["pass"].as<String>();

        provisioningSettings.certificates.clientCert = doc["tempCertPem"].as<String>();
        provisioningSettings.certificates.privateKey = doc["tempPrivateKey"].as<String>();
        
        break;

      }

    }
    
    DEBUG_PRINTLN("Disconnected from central device!");
    
  }

  return provisioningSettings;

}

bool BLEManager::disconnect() {

  BLEDevice central = BLE.central();
  bool success = central.disconnect();
  DEBUG_PRINTLN(success ? "Bluetooth disconnected from central device" :
    "Failed to disconnect Bluetooth from central device");

  return success;

}