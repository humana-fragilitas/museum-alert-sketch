#include "provisioning.h"

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
    }), m_onComplete{onComplete} {}

void Provisioning::registerDevice(Certificates certificates) {

  DEBUG_PRINTLN("Registering device; waiting for TSL certificates...");

  mqttClient.connect(certificates.clientCert, certificates.privateKey, "");

  mqttClient.subscribe(MqttEndpoints::AWS_CERTIFICATES_PROVISIONING_RESPONSE_TOPIC);
  mqttClient.publish(MqttEndpoints::AWS_CERTIFICATES_PROVISIONING_TOPIC, "");
    
}

void Provisioning::onResponse(const char topic[], byte* payload, unsigned int length) {
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';

    DEBUG_PRINTF("Received a message on topic '%s'\n", topic);
    DEBUG_PRINTLN(message);

    if (strcmp(topic, MqttEndpoints::AWS_CERTIFICATES_PROVISIONING_RESPONSE_TOPIC) == 0) {
        DEBUG_PRINTLN("Received TLS certificates; registering device...");
        this->onCertificates(message);
    } else if (strcmp(topic, MqttEndpoints::AWS_DEVICE_PROVISIONING_RESPONSE_TOPIC) == 0) {
        DEBUG_PRINTLN("Received device registration response");
        this->onDeviceRegistered(message);
    } else {
        DEBUG_PRINTF("Topic '%s' not handled\n", topic);
    }
}

void Provisioning::onCertificates(const char* message) {

    JsonDocument response;
    DeserializationError error = deserializeJson(response, message);
    if (error) {
        DEBUG_PRINTF("Failed to deserialize device provisioning certificates JSON: %s\n", error.c_str());
        m_onComplete(false);
        return;
    }

    const char* certificatePem = response["certificatePem"];
    const char* privateKey = response["privateKey"];

    snprintf(tempCertificates.clientCert, sizeof(Certificates::CERT_SIZE), certificatePem);
    snprintf(tempCertificates.privateKey, sizeof(Certificates::KEY_SIZE), privateKey);

    if (!tempCertificates.isValid()) {
        DEBUG_PRINTLN("Did not receive valid certificates: exiting provisioning flow...");
        m_onComplete(false);
        return;
    }

    JsonDocument deviceRegistrationPayload;
    deviceRegistrationPayload["certificateOwnershipToken"] = response["certificateOwnershipToken"];
    JsonObject parameters = deviceRegistrationPayload["parameters"].to<JsonObject>();
    parameters["ThingName"] = Sensor::name;
    parameters["Company"] = "ACME"; // TODO: make this data dynamic

    char deviceRegistrationPayloadJsonString[256];
    serializeJson(deviceRegistrationPayload, deviceRegistrationPayloadJsonString);

    DEBUG_PRINTLN("Attempting to register device with the following payload:");
    DEBUG_PRINTLN(deviceRegistrationPayloadJsonString);

    mqttClient.subscribe(MqttEndpoints::AWS_DEVICE_PROVISIONING_RESPONSE_TOPIC);
    mqttClient.publish(MqttEndpoints::AWS_DEVICE_PROVISIONING_TOPIC, deviceRegistrationPayloadJsonString);

}

void Provisioning::onDeviceRegistered(const char* message) {

    JsonDocument response;
    DeserializationError error = deserializeJson(response, message);

    if (error) {
        DEBUG_PRINTF("Failed to deserialize device registration response JSON: %s\n", error.c_str());
        DEBUG_PRINTLN("Exiting provisioning flow...");
        m_onComplete(false);
        return;
    }

    if (strcmp(response["thingName"], Sensor::name) != 0) {
        DEBUG_PRINTLN("Failed to register device: exiting provisioning flow...");
        m_onComplete(false);
        return;
    }

    m_onComplete(true);
}
