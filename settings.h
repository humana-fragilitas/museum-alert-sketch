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
  DEVICE_INITIALIZED,
  FATAL_ERROR
};

namespace Timing {
  static constexpr unsigned int SERIAL_PORT_INIT_TIMEOUT_MS = CONF_SERIAL_PORT_INIT_TIMEOUT_MS;
  static constexpr unsigned int LED_INDICATORS_STATE_INTERVAL_MS = CONF_LED_INDICATORS_STATE_INTERVAL_MS;
  static constexpr unsigned int FREE_HEAP_MEMORY_DEBUG_LOG_INTERVAL_MS = CONF_FREE_HEAP_MEMORY_DEBUG_LOG_INTERVAL_MS;
  static constexpr unsigned int DEVICE_CONFIGURATION_SCAN_INTERVAL_MS = CONF_DEVICE_CONFIGURATION_SCAN_INTERVAL_MS;
  static constexpr unsigned int SENSOR_DETECTION_INTERVAL_MS = CONF_SENSOR_DETECTION_INTERVAL_MS;
  static constexpr unsigned int DEBUG_FORCED_INITIALIZATION_DELAY_MS = CONF_DEBUG_FORCED_INITIALIZATION_DELAY_MS;
  static constexpr unsigned int WIFI_AUTO_CONNECTION_TIMEOUT_MS = CONF_WIFI_AUTO_CONNECTION_TIMEOUT_MS;
  static constexpr unsigned int DEVICE_CONTROLS_PROCESSOR_INTERVAL_MS = CONF_DEVICE_CONTROLS_PROCESSOR_INTERVAL_MS;
  static constexpr unsigned int USB_COMMANDS_SCAN_INTERVAL_MS = CONF_USB_COMMANDS_SCAN_INTERVAL_MS;
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
  static constexpr char AWS_CERTIFICATES_PROVISIONING_REJECTED_RESPONSE_TOPIC[] = CONF_AWS_DEVICE_PROVISIONING_REJECTED_RESPONSE_TOPIC;
  static constexpr char AWS_DEVICE_PROVISIONING_TOPIC[] = CONF_AWS_DEVICE_PROVISIONING_TOPIC;
  static constexpr char AWS_DEVICE_PROVISIONING_RESPONSE_TOPIC[] = CONF_AWS_DEVICE_PROVISIONING_RESPONSE_TOPIC;
  static constexpr char DEVICE_INCOMING_COMMANDS_TOPIC[] = CONF_DEVICE_INCOMING_COMMANDS_TOPIC_TEMPLATE;
  static constexpr char DEVICE_OUTGOING_COMMANDS_ACK_TOPIC_TEMPLATE[] = CONF_DEVICE_OUTGOING_COMMANDS_ACK_TOPIC_TEMPLATE;
  static constexpr char DEVICE_OUTGOING_DATA_TOPIC[] = CONF_DEVICE_OUTGOING_DATA_TOPIC_TEMPLATE;
};

namespace Encryption {
  static constexpr size_t KEY_SIZE = 16;
  static constexpr size_t AES_BLOCK_SIZE = 16;
  static constexpr size_t MAX_PAYLOAD_SIZE = 256;
};

namespace Storage {
  static constexpr char NAME[] = CONF_STORAGE_NAME;
  static constexpr char COMPANY_NAME_LABEL[] = CONF_COMPANY_NAME_LABEL;
  static constexpr char CLIENT_CERT_LABEL[] = CONF_STORAGE_CLIENT_CERT_LABEL;
  static constexpr char PRIVATE_KEY_LABEL[] = CONF_STORAGE_PRIVATE_KEY_LABEL;
  static constexpr char DISTANCE_LABEL[] = CONF_STORAGE_DISTANCE_LABEL;
  static constexpr char ENCRYPTION_KEY_LABEL[] = CONF_STORAGE_ENCRYPTION_KEY_LABEL;
}

// Outgoing messages: from device to app
enum MqttMessageType {
  ALARM = 100,
  // note: connection status is automatically
  // sent via AWS IoT Core default topics and forwarded
  // to company-specific events via AWS IoT rule and
  // associated lambda function
  CONNECTION_STATUS = 101,
  // note: message type originated from
  // a command of type GET_CONFIGURATION
  // see MqttCommandType enum
  CONFIGURATION = 102,
  // signals positive reception of command which is not
  // required to respond with a data payload
  ACKNOWLEDGMENT = 103
};

// Incoming commands: from app to device
enum MqttCommandType {
  RESET = 200,
  GET_CONFIGURATION = 201,
  SET_CONFIGURATION = 202
};

enum USBMessageType {
  APP_STATE,
  WIFI_NETWORKS_LIST,
  ERROR
};

enum USBCommandType {
  HARD_RESET,
  // Add more commands here
  USB_COMMAND_TYPE_COUNT,
  USB_COMMAND_INVALID = -1
};

enum ErrorType {
  NONE,
  CIPHERING_INITIALIZATION_ERROR,
  INVALID_WIFI_CREDENTIALS,
  FAILED_WIFI_CONNECTION_ATTEMPT,
  INVALID_DEVICE_PROVISIONING_SETTINGS,
  FAILED_PROVISIONING_SETTINGS_STORAGE,
  FAILED_DEVICE_PROVISIONING_ATTEMPT,
  FAILED_MQTT_BROKER_CONNECTION,
  FAILED_DEVICE_CONFIGURATION_RETRIEVAL,
  FAILED_SENSOR_DETECTION_REPORT
};

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
  String idToken;

  Certificates() { clear(); }

  bool isValid() const {
    return !clientCert.isEmpty() &&
           !privateKey.isEmpty();
  }

  void clear() {
    clientCert.clear();
    privateKey.clear();
    idToken.clear();
  }
  
};

struct DeviceConfiguration {

  Certificates certificates;
  String companyName;

  DeviceConfiguration() { clear(); }

  bool isValid() const {
  return certificates.isValid() &&
         !companyName.isEmpty();
  }

  void clear() {
    certificates.clear();
    companyName.clear();
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
