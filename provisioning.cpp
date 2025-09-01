#include "provisioning.h"       

Provisioning::Provisioning(std::function<void(bool, const AwsIotConfiguration&)> onComplete) :
   mqttClient([&](const char topic[], byte* payload, unsigned int length) {
     this->onResponse(topic, payload, length);
   }), m_onComplete{onComplete} {}

void Provisioning::registerDevice(const Certificates& certificates) {

  idToken = certificates.idToken;

  DEBUG_PRINTLN("Registering device; waiting for TSL certificates...");

  mqttClient.connect(certificates.clientCert, certificates.privateKey, "PROVISIONING-CLIENT");

  DEBUG_PRINTF("CERTIFICATES... %s, %s", certificates.clientCert.c_str(), certificates.privateKey.c_str());

  mqttClient.subscribe(MqttEndpoints::DEVICE_PROVISIONING_REJECTED_RESPONSE_TOPIC);
  mqttClient.subscribe(MqttEndpoints::CERTIFICATES_PROVISIONING_RESPONSE_TOPIC);
  mqttClient.publish(MqttEndpoints::CERTIFICATES_PROVISIONING_TOPIC, "");
   
}

void Provisioning::onResponse(const char topic[], byte* payload, unsigned int length) {

    DEBUG_PRINTF("Received message on topic: %s; length: %d\n", topic, length);  

   auto* message = static_cast<char*>(malloc(length + 1));
   if (!message) {
     DEBUG_PRINTLN("Memory allocation failed!");
     return;
   }

   // Copy payload and null-terminate
   memcpy(message, payload, length);
   message[length] = '\0';

   DEBUG_PRINTF("Received a message on topic '%s'\n", topic);
   DEBUG_PRINTLN(message);

   if (strcmp(topic, MqttEndpoints::CERTIFICATES_PROVISIONING_RESPONSE_TOPIC) == 0) {
     DEBUG_PRINTLN("Received TLS certificates; registering device...");
     this->onCertificates(message, true);
   } else if (strcmp(topic, MqttEndpoints::DEVICE_PROVISIONING_REJECTED_RESPONSE_TOPIC) == 0) {
     DEBUG_PRINTLN("An error prevented the certificates from being generated; exiting...");
     this->onCertificates(message, false);
   } else if (strcmp(topic, MqttEndpoints::DEVICE_PROVISIONING_RESPONSE_TOPIC) == 0) {
     DEBUG_PRINTLN("Received device registration response");
     this->onDeviceRegistered(message);
   } else {
     DEBUG_PRINTF("Topic '%s' not handled\n", topic);
   }

   free(message);

}

void Provisioning::onCertificates(const char* message, bool success) {

   JsonDocument response;
   const auto error = deserializeJson(response, message);

   if (error) {
     DEBUG_PRINTF("Failed to deserialize device provisioning certificates JSON: %s\n", error.c_str());
     m_onComplete(false, configuration);
     return;
   }

   if (!success) {
     DEBUG_PRINTLN("Cannot proceed to register device without valid certificates");
     m_onComplete(false, configuration);
     return;
   }

   DEBUG_PRINTF("AWS device TLS certificate and private key provisioning response: %s\n", message);

   const auto certificatePem = response["certificatePem"].as<String>();
   const auto privateKey = response["privateKey"].as<String>();

   configuration.certificates.clientCert = certificatePem;
   configuration.certificates.privateKey = privateKey;

   if (!configuration.certificates.isValid()) {
     DEBUG_PRINTLN("Did not receive valid certificates: exiting provisioning flow...");
     m_onComplete(false, configuration);
     return;
   }

   JsonDocument deviceRegistrationPayload;
   deviceRegistrationPayload["certificateOwnershipToken"] = response["certificateOwnershipToken"];
   auto parameters = deviceRegistrationPayload["parameters"].to<JsonObject>();
   parameters["ThingName"] = Sensor::name;
   parameters["idToken"] = idToken;

   String deviceRegistrationPayloadJsonString;
   serializeJson(deviceRegistrationPayload, deviceRegistrationPayloadJsonString);

   DEBUG_PRINTLN("Attempting to register device with the following payload:");
   DEBUG_PRINTLN(deviceRegistrationPayloadJsonString);

   mqttClient.subscribe(MqttEndpoints::DEVICE_PROVISIONING_RESPONSE_TOPIC);
   mqttClient.publish(MqttEndpoints::DEVICE_PROVISIONING_TOPIC, deviceRegistrationPayloadJsonString.c_str());

}

void Provisioning::onDeviceRegistered(const char* message) {

   JsonDocument response;
   const auto error = deserializeJson(response, message);

   if (error) {
     DEBUG_PRINTF("Failed to deserialize device registration response JSON: %s\n", error.c_str());
     DEBUG_PRINTLN("Exiting provisioning flow...");
     m_onComplete(false, configuration);
     return;
   }

   DEBUG_PRINTF("AWS device registration response: %s\n", message);

   const char* thingName = response["thingName"];
   const char* companyName = response["deviceConfiguration"]["company"];

   if ((strcmp(thingName, Sensor::name) != 0) ||
     (companyName == nullptr || strlen(companyName) == 0)) {
     DEBUG_PRINTLN("Failed to register device: exiting provisioning flow...");
     m_onComplete(false, configuration);
     return;
   }

   configuration.companyName = String(companyName);

   m_onComplete(true, configuration);
   
}
