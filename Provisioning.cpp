#include "Provisioning.h"
#include "Sensor.h"

  /* Certificates provisioning response example
  {
    "certificateId":"<CERTIFICATE_ID>",
    "certificatePem":"-----BEGIN CERTIFICATE-----<CERTIFICATE>\n-----END CERTIFICATE-----\n",
    "privateKey":"-----BEGIN RSA PRIVATE KEY-----<PRIVATE_KEY>\n-----END RSA PRIVATE KEY-----\n",
    "certificateOwnershipToken":"<CERTIFICATE_OWNERSHIP_TOKEN>"
  }
  */

  /* Device registration payload

  {
    "certificateOwnershipToken": "<TOKEN HERE>",
    "parameters": {
        "ThingName": "MAS-999999999",
        "Company": "ACME"
    }
  }

  */

  /* Device registration response

  // Success

  {"deviceConfiguration":{},"thingName":"MAS-999999999"}

  // Error

  {"statusCode":400,"errorCode":"InvalidParameters","errorMessage":"Cannot resolve reference value: AWS::Region"}

  */

Provisioning::Provisioning(std::function<void(bool)> onComplete) :
  mqttClient([&](const char topic[], byte* payload, unsigned int length) {

    this->onResponse(topic, payload, length);

  }), m_onComplete{onComplete} { }

void Provisioning::registerDevice(Certificates certificates) {

  mqttClient.connect(certificates.clientCert.c_str(), certificates.privateKey.c_str(), "");

  mqttClient.subscribe(MqttEndpoints::certificatesProvisioningResponseTopic);
  mqttClient.publish(MqttEndpoints::certificatesProvisioningTopic.c_str(), ""); // TO DO: subscribe and publish methods should both accept String type

  Serial.println("Registering device; waiting for TSL certificates...\n");

}

void Provisioning::onResponse(const char topic[], byte* payload, unsigned int length) {

    String message = String((char*)payload).substring(0, length);

    Serial.printf("Received a message on topic '%s'\n", topic);
    Serial.println(message);

  if (strcmp(topic, MqttEndpoints::certificatesProvisioningResponseTopic.c_str()) == 0) {

    Serial.println("Received TLS certificates; registering device...");
    this->onCertificates(message);

  } else if (strcmp(topic, MqttEndpoints::deviceProvisioningResponseTopic.c_str()) == 0) {

    Serial.println("Received device registration response");
    this->onDeviceRegistered(message);

  } else {

      Serial.printf("Topic '%s' not handled\n", topic);

  }

}

void Provisioning::onCertificates(String message) {

  JsonDocument response;
  DeserializationError error = deserializeJson(response, message);
  if (error) {
    Serial.printf("Failed to deserialize device provisioning certificates json: %s\n", error.c_str());
    m_onComplete(false);
    return;
  }

  tempCertificates.clientCert = response["certificatePem"].as<String>();
  tempCertificates.privateKey = response["privateKey"].as<String>();

  if (!tempCertificates.isValid()) {
    Serial.println("Did not receive valid certificates: exiting provisioning flow...");
    m_onComplete(false);
    return;
  }

  JsonDocument deviceRegistrationPayload;
  deviceRegistrationPayload["certificateOwnershipToken"] = response["certificateOwnershipToken"];
  JsonObject parameters = deviceRegistrationPayload["parameters"].to<JsonObject>();
  parameters["ThingName"] = Sensor::sensorName;
  parameters["Company"] = "ACME"; // TO DO: make this data dynamic

  String deviceRegistrationPayloadJsonString;
  serializeJson(deviceRegistrationPayload, deviceRegistrationPayloadJsonString);

  Serial.println("Attempting to register device with the following payload:");
  Serial.println(deviceRegistrationPayloadJsonString);

  mqttClient.subscribe(MqttEndpoints::deviceProvisioningResponseTopic);
  mqttClient.publish(MqttEndpoints::deviceProvisioningTopic.c_str(), deviceRegistrationPayloadJsonString.c_str());

}

void Provisioning::onDeviceRegistered(String message) {

  JsonDocument response;
  DeserializationError error = deserializeJson(response, message);

  if (error) {
    Serial.printf("Failed to deserialize device provisioning certificates json: %s\n", error.c_str());
    Serial.println("Exiting provisioning flow...");
    m_onComplete(false);
    return;
  }

  if (response["thingName"].as<String>() != Sensor::sensorName) {
    Serial.println("Failed to register device: exiting provisioning flow...");
    m_onComplete(false);
    return;
  }

  certManager.storeCertificates(tempCertificates);

  m_onComplete(true);
  
}
