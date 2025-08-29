#include "json_helper.h"


template<>
WiFiCredentials JsonHelper::parse<WiFiCredentials>(const String& json) {

   WiFiCredentials request{};
   JsonDocument doc;
   DeserializationError error = deserializeJson(doc, json);
   
   if (error) {
     DEBUG_PRINTF("Failed to deserialize WiFi credentials request JSON: %s\n", error.c_str());
     return request;
   }
   
   request.ssid = doc["ssid"].as<String>();
   request.password = doc["password"].as<String>();
   
   return request;

}

template<> 
Certificates JsonHelper::parse<Certificates>(const String& json) {

  Certificates certificates{};
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);
  
  if (error) {
     DEBUG_PRINTF("Failed to deserialize provisioning certificates JSON: %s\n", error.c_str());
     return certificates;
   }

  DEBUG_PRINTLN("Deserializing provisiong settings JSON");

  String tempCertPem = doc["tempCertPem"].as<String>();
  String tempPrivateKey = doc["tempPrivateKey"].as<String>();
  String idToken = doc["idToken"].as<String>();

  certificates.clientCert = tempCertPem;
  certificates.privateKey = tempPrivateKey;
  certificates.idToken = idToken;

  return certificates;

}