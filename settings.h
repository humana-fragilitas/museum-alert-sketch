#ifndef SETTINGS
#define SETTINGS

#include "macros.h"
#include "config.h"

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
  static constexpr unsigned int LED_INDICATORS_STATE_MS = 250;
  static constexpr unsigned int FREE_HEAP_MEMORY_DEBUG_LOG_MS = 10000;
  static constexpr unsigned int WIFI_NETWORKS_SCAN_MS = 4000;
  static constexpr unsigned int SENSOR_DETECTION_MS = 4000;
  static constexpr unsigned int DEBUG_FORCED_INITIALIZATION_DELAY = 20000;
};

namespace MqttEndpoints {
  static constexpr char AWS_IOT_CORE_ENDPOINT[] = CONF_AWS_IOT_CORE_ENDPOINT;
  static constexpr char AWS_CERTIFICATES_PROVISIONING_TOPIC[] = CONF_AWS_CERTIFICATES_PROVISIONING_TOPIC;
  static constexpr char AWS_CERTIFICATES_PROVISIONING_RESPONSE_TOPIC[] = CONF_AWS_CERTIFICATES_PROVISIONING_RESPONSE_TOPIC;
  static constexpr char AWS_DEVICE_PROVISIONING_TOPIC[] = CONF_AWS_DEVICE_PROVISIONING_TOPIC;
  static constexpr char AWS_DEVICE_PROVISIONING_RESPONSE_TOPIC[] = CONF_AWS_DEVICE_PROVISIONING_RESPONSE_TOPIC;
  static constexpr char DEVICE_INCOMING_COMMANDS_TOPIC[] = CONF_DEVICE_INCOMING_COMMANDS_TOPIC_TEMPLATE;
  static constexpr char DEVICE_OUTGOING_DATA_TOPIC[] = CONF_DEVICE_OUTGOING_DATA_TOPIC_TEMPLATE;
};

namespace Encryption {
  static constexpr size_t KEY_SIZE = 16;
  static constexpr size_t AES_BLOCK_SIZE = 16;
  static constexpr size_t MAX_PAYLOAD_SIZE = 256;
};

namespace Storage {
  static constexpr char NAME[] = "STORAGE";
  static constexpr char CLIENT_CERT_LABEL[] = "CLIENT_CERT";
  static constexpr char PRIVATE_KEY_LABEL[] = "PRIVATE_KEY";
  static constexpr char ENCRYPTION_KEY_LABEL[] = "ENCRYPTION_KEY";
}

struct WiFiCredentials {

  static constexpr size_t SSID_SIZE = 64;
  static constexpr size_t PASSWORD_SIZE = 128;
  char ssid[SSID_SIZE];
  char password[PASSWORD_SIZE];

  WiFiCredentials() { clear(); }

  bool isValid() const {
    return ssid[0] != '\0' && password[0] != '\0';
  }

  void clear() {
    ssid[0] = '\0';
    password[0] = '\0';
  }

};

struct Certificates {

  static constexpr size_t CERT_SIZE = 4096;
  static constexpr size_t KEY_SIZE = 4096;
  char clientCert[CERT_SIZE];
  char privateKey[KEY_SIZE];

  Certificates() { clear(); }

  bool isValid() const {
    return clientCert[0] != '\0' && privateKey[0] != '\0';
  }

  void clear() {
    clientCert[0] = '\0';
    privateKey[0] = '\0';
  }
  
};

struct ProvisioningSettings {

  WiFiCredentials wiFiCredentials;
  Certificates certificates;

  ProvisioningSettings() { clear(); }  // Constructor initializes the struct

  bool isValid() const {
    return wiFiCredentials.isValid() && certificates.isValid();
  }

  void clear() {
    wiFiCredentials.clear();
    certificates.clear();
  }

};

#endif
