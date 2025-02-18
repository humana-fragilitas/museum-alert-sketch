#ifndef SETTINGS
#define SETTINGS

#include "macros.h"
#include "config.h"

enum AppState {
  STARTED,
  INITIALIZE_CIPHERING,
  CONFIGURE_WIFI,
  CONFIGURE_CERTIFICATES,
  CONNECT_TO_WIFI,
  PROVISION_DEVICE,
  CONNECT_TO_MQTT_BROKER,
  DEVICE_INITIALIZED
};

namespace Timing {
  static constexpr unsigned int SERIAL_PORT_INIT_TIMEOUT_MS = CONF_SERIAL_PORT_INIT_TIMEOUT_MS;
  static constexpr unsigned int LED_INDICATORS_STATE_INTERVAL_MS = CONF_LED_INDICATORS_STATE_INTERVAL_MS;
  static constexpr unsigned int FREE_HEAP_MEMORY_DEBUG_LOG_INTERVAL_MS = CONF_FREE_HEAP_MEMORY_DEBUG_LOG_INTERVAL_MS;
  static constexpr unsigned int DEVICE_CONFIGURATION_SCAN_INTERVAL_MS = CONF_DEVICE_CONFIGURATION_SCAN_INTERVAL_MS;
  static constexpr unsigned int SENSOR_DETECTION_INTERVAL_MS = CONF_SENSOR_DETECTION_INTERVAL_MS;
  static constexpr unsigned int DEBUG_FORCED_INITIALIZATION_DELAY_MS = CONF_DEBUG_FORCED_INITIALIZATION_DELAY_MS;
  static constexpr unsigned int WIFI_AUTO_CONNECTION_TIMEOUT_MS = CONF_WIFI_AUTO_CONNECTION_TIMEOUT_MS;
};

namespace Communication {
  static constexpr unsigned int SERIAL_COM_BAUD_RATE = CONF_SERIAL_COM_BAUD_RATE;
}

namespace Bluetooth {
  static constexpr char DEVICE_SERVICE_UUID[] = CONF_DEVICE_SERVICE_UUID;
  static constexpr char DEVICE_SERVICE_CONFIGURATION_CHARACTERISTIC_UIID[] = CONF_DEVICE_SERVICE_CONFIGURATION_CHARACTERISTIC_UIID;
  static constexpr char DEVICE_SERVICE_SSIDS_CHARACTERISTIC_UIID[] = CONF_DEVICE_SERVICE_SSIDS_CHARACTERISTIC_UIID;
}

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
  static constexpr char NAME[] = CONF_STORAGE_NAME;
  static constexpr char CLIENT_CERT_LABEL[] = CONF_STORAGE_CLIENT_CERT_LABEL;
  static constexpr char PRIVATE_KEY_LABEL[] = CONF_STORAGE_PRIVATE_KEY_LABEL;
  static constexpr char ENCRYPTION_KEY_LABEL[] = CONF_STORAGE_ENCRYPTION_KEY_LABEL;
}

struct WiFiCredentials {

  String ssid;
  String password;

  WiFiCredentials() { clear(); }

  bool isValid() const {
    return !ssid.isEmpty() && !password.isEmpty();
  }

  void clear() {
    ssid.clear();
    password.clear();
  }

};

struct Certificates {

  String clientCert;
  String privateKey;

  Certificates() { clear(); }

  bool isValid() const {
    return !clientCert.isEmpty() && !privateKey.isEmpty();
  }

  void clear() {
    clientCert.clear();
    privateKey.clear();
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
