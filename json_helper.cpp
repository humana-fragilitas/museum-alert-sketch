#include "json_helper.h"

template<>
DeviceCommandRequest JsonHelper::parse<DeviceCommandRequest>(const String& json) {

  DeviceCommandRequest request;
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);

  request.correlationId = doc["cid"].as<String>();

  if (error || !doc["command"].is<int>()) {
    request.payload = USB_COMMAND_INVALID;
    return request;
  }

  int commandId = doc["command"].as<int>();

  if (commandId < 0 || commandId >= USB_COMMAND_TYPE_COUNT) {
    request.payload = USB_COMMAND_INVALID;
  } else {
    request.payload = static_cast<USBCommandType>(commandId);
  }

  return request;

}

template<>
WiFiCredentialsRequest JsonHelper::parse<WiFiCredentialsRequest>(const String& json) {

    WiFiCredentialsRequest request;
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);
    
    if (error) {
        DEBUG_PRINTF("Failed to deserialize WiFi credentials request JSON: %s\n", error.c_str());
        return request;
    }
    
    request.correlationId = doc["cid"].as<String>();
    
    request.payload.ssid = doc["ssid"].as<String>();
    request.payload.password = doc["password"].as<String>();
    
    return request;

}

template<>
CertificatesRequest JsonHelper::parse<CertificatesRequest>(const String& json) {

    CertificatesRequest request;
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);
    
    if (error) {
        DEBUG_PRINTF("Failed to deserialize provisioning certificates JSON: %s\n", error.c_str());
        return request;
      }

    DEBUG_PRINTLN("Deserializing provisiong settings JSON");

    String tempCertPem = doc["tempCertPem"].as<String>();
    String tempPrivateKey = doc["tempPrivateKey"].as<String>();
    String idToken = doc["idToken"].as<String>();

    request.correlationId = doc["cid"].as<String>();

    request.payload.clientCert = tempCertPem;
    request.payload.privateKey = tempPrivateKey;
    request.payload.idToken = idToken;

    return request;

}