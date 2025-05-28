#include "provisioning.h"

// AWS Relevant documentation: https://docs.aws.amazon.com/iot/latest/developerguide/fleet-provision-api.html
// Exclusive certificate: https://docs.aws.amazon.com/iot/latest/developerguide/attach-thing-principal.html
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

Provisioning::Provisioning(std::function<void(bool, DeviceConfiguration)> onComplete) :
    mqttClient([&](const char topic[], byte* payload, unsigned int length) {
        this->onResponse(topic, payload, length);
    }), m_onComplete{onComplete} {}

void Provisioning::registerDevice(Certificates certificates) {

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

    // Allocate memory on the heap
    char* message = (char*)malloc(length + 1);
    if (!message) {
        DEBUG_PRINTLN("Memory allocation failed!");
        return; // Exit function if allocation fails
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

    // Free allocated memory to avoid leaks
    free(message);
}

void Provisioning::onCertificates(const char* message, bool success) {

    JsonDocument response;
    DeserializationError error = deserializeJson(response, message);

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

    String certificatePem = response["certificatePem"].as<String>();
    String privateKey = response["privateKey"].as<String>();

    configuration.certificates.clientCert = certificatePem;
    configuration.certificates.privateKey = privateKey;

    if (!configuration.certificates.isValid()) {
      DEBUG_PRINTLN("Did not receive valid certificates: exiting provisioning flow...");
      m_onComplete(false, configuration);
      return;
    }

    JsonDocument deviceRegistrationPayload;
    deviceRegistrationPayload["certificateOwnershipToken"] = response["certificateOwnershipToken"];
    JsonObject parameters = deviceRegistrationPayload["parameters"].to<JsonObject>();
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
    DeserializationError error = deserializeJson(response, message);

    if (error) {
        DEBUG_PRINTF("Failed to deserialize device registration response JSON: %s\n", error.c_str());
        DEBUG_PRINTLN("Exiting provisioning flow...");
        m_onComplete(false, configuration);
        return;
    }

    DEBUG_PRINTF("AWS device registration response: %s\n", message);

    /*
       Sample expected response: {"deviceConfiguration":{"company":"acme"},"thingName":"MAS-EC357A188534"}
    */

    const char* thingName = response["thingName"];
    const char* companyName = response["deviceConfiguration"]["company"];

    if ((strcmp(thingName, Sensor::name) != 0) ||
        (companyName == nullptr || strlen(companyName) == 0)) {
        DEBUG_PRINTLN("Failed to register device: exiting provisioning flow...");
        m_onComplete(false, configuration);
        return;
    }

    configuration.companyName = String(companyName);

    // return company here together with certificates
    m_onComplete(true, configuration);
    
}
