#ifndef CONFIG
#define CONFIG


/******************************************************************************
 * MACROS                                                                     *
 ******************************************************************************/

/**
  * Uncomment/comment the following line to enable debugging mode
  * Alternatively, enable debugging via Arduino CLI: arduino-cli compile --fqbn esp32:esp32:esp32 --build-property build.extra_flags=-DDEBUG
  */
#define DEBUG 

#ifdef DEBUG
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(...)
#endif


/******************************************************************************
 * CORE APPLICATION TYPES                                                     *
 ******************************************************************************/

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
  ACK = 103 // TO DO: conflicts with USBMessageType;

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
  ERROR,
  ACKNOWLEDGMENT

};

enum USBCommandType {

  SET_PROVISIONING_CERTIFICATES,
  REFRESH_WIFI_CREDENTIALS,
  SET_WIFI_CREDENTIALS,
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
  INVALID_DEVICE_COMMAND,
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

struct RequestWrapper {
    String correlationId;
    USBCommandType commandType;
    String payloadJson;
    
    RequestWrapper() : commandType(USB_COMMAND_INVALID) {}
    
    RequestWrapper(const String& corrId, USBCommandType cmdType, const String& payload = "") 
        : correlationId(corrId), commandType(cmdType), payloadJson(payload) {}
    
    bool hasPayload() const {
        return payloadJson.length() > 0 && payloadJson != "null";
    }
};

// TO DO: rename to AwsIotConfiguration?
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

/******************************************************************************
 * CONFIGURATION                                                              *
 ******************************************************************************/

namespace Pins {

  constexpr int Trigger = 9;
  constexpr int Echo = 8;
  constexpr int ResetButton = 7;
  constexpr int WiFi = 4;
  constexpr int Status = 3;
  constexpr int Alarm = 2;

}

namespace Configuration {

  static constexpr const char* FIRMWARE_VERSION = "1.0.0";
  static constexpr float DEFAULT_ALARM_DISTANCE = 30.0;
  static constexpr float MINIMUM_ALARM_DISTANCE = 5.0;
  static constexpr float MAXIMUM_ALARM_DISTANCE = 500.0;
  static constexpr int MAXIMUM_BLE_BEACON_ENCODED_URL_LENGTH = 18;

}

namespace Timing {

  static constexpr unsigned int SERIAL_PORT_INIT_TIMEOUT_MS = 2000;
  static constexpr unsigned int LED_INDICATORS_STATE_INTERVAL_MS = 250;
  static constexpr unsigned int FREE_HEAP_MEMORY_DEBUG_LOG_INTERVAL_MS = 10000;
  static constexpr unsigned int DEVICE_CONFIGURATION_SCAN_INTERVAL_MS = 4000;
  static constexpr unsigned int SENSOR_DETECTION_INTERVAL_MS = 4000;
  static constexpr unsigned int BEACON_MAINTENANCE_INTERVAL_MS = 30000;
  static constexpr unsigned int BEACON_MIN_INTERVAL_UNITS = 160;  // 0.625ms units
  static constexpr unsigned int BEACON_MAX_INTERVAL_UNITS = 320;  // 0.625ms units
  static constexpr unsigned int DEBUG_FORCED_INITIALIZATION_DELAY_MS = 20000;
  static constexpr unsigned int WIFI_AUTO_CONNECTION_TIMEOUT_MS = 5000;
  static constexpr unsigned int DEVICE_CONTROLS_PROCESSOR_INTERVAL_MS = 250;
  static constexpr unsigned int USB_COMMANDS_SCAN_INTERVAL_MS = 1000;

};

namespace Communication {

  static constexpr unsigned int SERIAL_COM_BAUD_RATE = 9600;
  static constexpr unsigned int SERIAL_COM_BUFFER_SIZE = 10000;

}

namespace Bluetooth {

  static constexpr const char* DEVICE_SERVICE_UUID = "19b10000-e8f2-537e-4f6c-d104768a1214";
  static constexpr const char* DEVICE_SERVICE_CONFIGURATION_CHARACTERISTIC_UIID = "19b10001-e8f2-537e-4f6c-d104768a1214";
  static constexpr const char* DEVICE_SERVICE_SSIDS_CHARACTERISTIC_UIID = "19b10001-e8f2-537e-4f6c-d104768a1213";

}

namespace AWS {

  static constexpr const char* REGION = "eu-west-1";
  static constexpr const char* IOT_CORE_ACCOUNT_IDENTIFIER = "avo0w7o1tlck1";

  inline String getIoTCoreEndpoint() {
    return String(AWS::IOT_CORE_ACCOUNT_IDENTIFIER) + "-ats.iot." + AWS::REGION + ".amazonaws.com";
  }

}

#define DEVICE_PROVISIONING_TEMPLATE "museum-alert-provisioning-template"

namespace MqttEndpoints {

  static constexpr const char* CERTIFICATES_PROVISIONING_TOPIC = "$aws/certificates/create/json";
  static constexpr const char* CERTIFICATES_PROVISIONING_RESPONSE_TOPIC = "$aws/certificates/create/json/accepted";

  static constexpr const char* DEVICE_PROVISIONING_TOPIC = "$aws/provisioning-templates/"
                                                            DEVICE_PROVISIONING_TEMPLATE
                                                            "/provision/json";

  static constexpr const char* DEVICE_PROVISIONING_RESPONSE_TOPIC = "$aws/provisioning-templates/"
                                                                    DEVICE_PROVISIONING_TEMPLATE
                                                                    "/provision/json/accepted";

  static constexpr const char* DEVICE_PROVISIONING_REJECTED_RESPONSE_TOPIC = "$aws/provisioning-templates/"
                                                                             DEVICE_PROVISIONING_TEMPLATE
                                                                             "/provision/json/rejected";

  static constexpr const char* DEVICE_INCOMING_COMMANDS_TOPIC_TEMPLATE = "companies/%s/devices/%s/commands";
  static constexpr const char* DEVICE_OUTGOING_COMMANDS_ACK_TOPIC_TEMPLATE = "companies/%s/devices/%s/commands/ack";
  static constexpr const char* DEVICE_OUTGOING_DATA_TOPIC_TEMPLATE = "companies/%s/devices/%s/events";

};

namespace Encryption {

  static constexpr size_t KEY_SIZE = 16;
  static constexpr size_t AES_BLOCK_SIZE = 16;
  static constexpr size_t MAX_PAYLOAD_SIZE = 256;

};

namespace Storage {

  static constexpr const char* NAME = "STORAGE";
  static constexpr const char* COMPANY_NAME_LABEL = "COMPANY";
  static constexpr const char* CLIENT_CERT_LABEL = "CLIENT_CERT";
  static constexpr const char* PRIVATE_KEY_LABEL = "PRIVATE_KEY";
  static constexpr const char* DISTANCE_LABEL = "DISTANCE";
  static constexpr const char* BEACON_URL_LABEL = "BEACON_URL";
  static constexpr const char* ENCRYPTION_KEY_LABEL = "ENCRYPTION_KEY";

}

#endif
