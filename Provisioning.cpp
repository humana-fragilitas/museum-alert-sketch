#include "Provisioning.h"

Provisioning::Provisioning() :
  mqttClient([&](const char topic[], byte* payload, unsigned int length) {

    this->onResponse(topic, payload, length);

  }) { }

void Provisioning::registerDevice(Certificates certificates) {

  mqttClient.connect(certificates.clientCert.c_str(), certificates.privateKey.c_str(), "");

  mqttClient.subscribe(MqttEndpoints::certificatesProvisioningResponseTopic);
  mqttClient.publish(MqttEndpoints::certificatesProvisioningTopic.c_str(), "");

  Serial.println("Registering device\n");

}

void Provisioning::onResponse(const char topic[], byte* payload, unsigned int length) {

    String message = String((char*)payload).substring(0, length);

    Serial.printf("Received a message on topic '%s'\n", topic);
    Serial.println(message);

  if (strcmp(topic, MqttEndpoints::certificatesProvisioningResponseTopic.c_str()) == 0) {

    this->onCertificates(payload, length);

  } else if (strcmp(topic, MqttEndpoints::deviceProvisioningResponseTopic.c_str()) == 0) {

    this->onDeviceRegistered(payload, length);

  } else {

      Serial.printf("Topic '%s' not handled\n", topic);

  }

}

void Provisioning::onCertificates(byte *payload, unsigned int length) {

  String message = String((char*)payload).substring(0, length);

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, message);

  /* Response example
  {
    "certificateId":"<CERTIFICATE_ID>",
    "certificatePem":"-----BEGIN CERTIFICATE-----<CERTIFICATE>\n-----END CERTIFICATE-----\n",
    "privateKey":"-----BEGIN RSA PRIVATE KEY-----<PRIVATE_KEY>\n-----END RSA PRIVATE KEY-----\n",
    "certificateOwnershipToken":"<CERTIFICATE_OWNERSHIP_TOKEN>"
  }
  */

  // extract and temporarily store certificates in memory
  // then pass ownership token to the next call 

  if (error) {
    Serial.printf("Failed to deserialize WiFi configuration json: %s\n", error.c_str());
    // TO DO: call to callback here with false calue
  }

  mqttClient.subscribe(MqttEndpoints::deviceProvisioningResponseTopic);
  mqttClient.publish(MqttEndpoints::certificatesProvisioningTopic.c_str(), "");

  

}

void Provisioning::onDeviceRegistered(byte *payload, unsigned int length) {

  
}
