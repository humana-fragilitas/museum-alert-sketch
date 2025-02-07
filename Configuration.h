#include "Macros.h"

#ifndef CONFIGURATION
#define CONFIGURATION

enum AppState {
  STARTED,
  INITIALIZE_CIPHERING,
  INITIALIZE_BLE,
  CONFIGURE_DEVICE,
  CONNECT_TO_WIFI,
  PROVISION_DEVICE,
  CONNECT_TO_MQTT_BROKER,
  DEVICE_INITIALIZED
};

namespace Timing {
  const unsigned int LED_INDICATORS_STATE_MS = 250;
  const unsigned int FREE_HEAP_MEMORY_DEBUG_LOG_MS = 10000;
  const unsigned int WIFI_NETWORKS_SCAN_MS = 4000;
  const unsigned int SENSOR_DETECTION_MS = 4000;
  const unsigned int DEBUG_FORCED_INITIALIZATION_DELAY = 20000;
};

namespace MqttEndpoints {
  const String AWS_IOT_CORE_ENDPOINT = "avo0w7o1tlck1-ats.iot.eu-west-1.amazonaws.com"; // TO DO: make IoT Core endpoint configurable at build time
  const String AWS_CERTIFICATES_PROVISIONING_TOPIC = "$aws/certificates/create/json";
  const String AWS_CERTIFICATES_PROVISIONING_RESPONSE_TOPIC = "$aws/certificates/create/json/accepted";
  const String AWS_DEVICE_PROVISIONING_TOPIC = "$aws/provisioning-templates/museum-alert-provisioning-template/provision/json";
  const String AWS_DEVICE_PROVISIONING_RESPONSE_TOPIC = "$aws/provisioning-templates/museum-alert-provisioning-template/provision/json/accepted";
  const String DEVICE_INCOMING_COMMANDS_TOPIC = "arn:aws:iot:eu-west-1:767398097786:topic/%s/sub"; // TO DO: make region and account id configurable at build time
  const String DEVICE_OUTGOING_DATA_TOPIC = "arn:aws:iot:eu-west-1:767398097786:topic/%s/pub";
};

namespace Encryption {
    const size_t KEY_SIZE = 16; // 128-bit key
    const size_t AES_BLOCK_SIZE = 16; // AES block size is always 16 bytes
};

namespace Storage {
  const char NAME[] = "STORAGE";
  const char CLIENT_CERT_LABEL[] = "CLIENT_CERT";
  const char PRIVATE_KEY_LABEL[] = "PRIVATE_KEY";
  const char ENCRYPTION_KEY_LABEL[] = "ENCRYPTION_KEY";
}

struct WiFiCredentials {
  String ssid;
  String password;
  bool isValid() {
    return !ssid.isEmpty() && !password.isEmpty();
  };
  void clear() {
    ssid = "";
    password = "";
  };
};

struct Certificates {
  String clientCert;
  String privateKey;
  bool isValid() {
    return !clientCert.isEmpty() &&
           !privateKey.isEmpty();
  };
  void clear() {
    clientCert = "";
    privateKey = "";
  };
};

struct ProvisioningPayload {
  String certificateOwnershipToken;
  String thingName;
  String company;
  void clear() {
    certificateOwnershipToken = "";
    thingName = "";
    company = "";
  };
};

struct ProvisioningSettings {
  WiFiCredentials wiFiCredentials;
  Certificates certificates;
  bool isValid() {
    return wiFiCredentials.isValid() &&
           certificates.isValid();
  };
  void clear() {
    wiFiCredentials.clear();
    certificates.clear();
  };
};

#endif

