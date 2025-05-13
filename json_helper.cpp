#include "json_helper.h"

Certificates JsonHelper::parseProvisioningCertificates(const String& json) {

  Certificates provisioningCertificates;

  // TO DO: check if still required
  //Serial.setTimeout(10000);

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);

  if (error) {
    DEBUG_PRINTF("Failed to deserialize provisioning certificates JSON: %s\n", error.c_str());
    return provisioningCertificates;
  }

  DEBUG_PRINTLN("Deserializing provisiong settings JSON");

  String tempCertPem = doc["tempCertPem"].as<String>();
  String tempPrivateKey = doc["tempPrivateKey"].as<String>();
  String idToken = doc["idToken"].as<String>();

  provisioningCertificates.clientCert = tempCertPem;
  provisioningCertificates.privateKey = tempPrivateKey;
  provisioningCertificates.idToken = idToken;

  return provisioningCertificates;

}

WiFiCredentials JsonHelper::parseWiFiCredentials(const String& json) {

  WiFiCredentials wiFiCredentials;

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);

  if (error) {
    DEBUG_PRINTF("Failed to deserialize WiFi credentials JSON: %s\n", error.c_str());
    return wiFiCredentials;
  }

  String ssid = doc["ssid"].as<String>();
  String password = doc["password"].as<String>();

  wiFiCredentials.ssid = ssid;
  wiFiCredentials.password = password;

  return wiFiCredentials;

}

USBCommandType JsonHelper::parseUSBCommand(const String& jsonString) {

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, jsonString);

  if (error || !doc["command"].is<int>()) {
      return USB_COMMAND_INVALID;
  }

  int commandId = doc["command"].as<int>();

  if (commandId < 0 || commandId >= USB_COMMAND_TYPE_COUNT) {
    return USB_COMMAND_INVALID;
  }

  return static_cast<USBCommandType>(commandId);
  
}