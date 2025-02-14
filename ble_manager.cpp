#include "ble_manager.h"

const char* BLEManager::deviceServiceUuid = Bluetooth::DEVICE_SERVICE_UUID;
const char* BLEManager::deviceServiceConfigurationCharacteristicUuid = Bluetooth::DEVICE_SERVICE_CONFIGURATION_CHARACTERISTIC_UIID;
const char* BLEManager::deviceServiceSsidsCharacteristicUuid = Bluetooth::DEVICE_SERVICE_SSIDS_CHARACTERISTIC_UIID;
BLEService BLEManager::configurationService(deviceServiceUuid);
BLEStringCharacteristic BLEManager::wiFiSsidsCharacteristic(deviceServiceSsidsCharacteristicUuid, BLERead, 512);
BLEStringCharacteristic BLEManager::configurationCharacteristic(deviceServiceConfigurationCharacteristicUuid, BLERead | BLEWrite, 512);


BLEManager::BLEManager() {}

bool BLEManager::initializeDeviceConfigurationService() {

  const char *sensorName = Sensor::name;

  if (!BLE.begin()) {
    DEBUG_PRINTLN("Failed to start Bluetooth® Low Energy module");
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

ProvisioningSettings BLEManager::getDeviceConfiguration(const char *json) {

  DEBUG_PRINTLN("Discovering central device via Bluetooth®...");

  ProvisioningSettings provisioningSettings;
  BLEDevice central = BLE.central();
  delay(250);

  if (central) {

    String macStr = central.address();
    char macAddress[18];

    strncpy(macAddress, macStr.c_str(), sizeof(macAddress) - 1);
    macAddress[sizeof(macAddress) - 1] = '\0';

    DEBUG_PRINTF("Connected via Bluetooth® to central device with MAC address: %s\n", macAddress);

    while (central.connected()) {

      //wiFiSsidsCharacteristic.writeValue(json);

      if (configurationCharacteristic.written()) {
        
        char configBuffer[1024]; // TO DO: adjust memory usage
        size_t configLength = configurationCharacteristic.valueLength();

        if (configLength >= sizeof(configBuffer)) {
          DEBUG_PRINTLN("Configuration payload received via Bluetooth® is too large! Ignoring...");
          continue;
        }

        configurationCharacteristic.readValue(configBuffer, configLength);
        configBuffer[configLength] = '\0';

        DEBUG_PRINTF("Received configuration via Bluetooth®: %s\n", configBuffer);

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, configBuffer);

        if (error) {
          DEBUG_PRINTLN("Failed to deserialize WiFi configuration JSON:");
          DEBUG_PRINTLN(error.c_str());
          continue;
        }

        const char *ssid = doc["ssid"];
        const char *password = doc["pass"];
        const char *tempCertPem = doc["tempCertPem"];
        const char *tempPrivateKey = doc["tempPrivateKey"];

        WiFiCredentials wiFiCredentials;
        snprintf(wiFiCredentials.ssid, WiFiCredentials::SSID_SIZE, ssid);
        snprintf(wiFiCredentials.password, WiFiCredentials::PASSWORD_SIZE, password);

        Certificates certificates;
        snprintf(certificates.clientCert, Certificates::CERT_SIZE, tempCertPem);
        snprintf(certificates.privateKey, Certificates::KEY_SIZE, tempPrivateKey);

        provisioningSettings.wiFiCredentials = wiFiCredentials;
        provisioningSettings.certificates = certificates;

      }

    }

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