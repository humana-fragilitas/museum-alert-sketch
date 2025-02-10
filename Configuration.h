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
  static constexpr unsigned int LED_INDICATORS_STATE_MS = 250;
  static constexpr unsigned int FREE_HEAP_MEMORY_DEBUG_LOG_MS = 10000;
  static constexpr unsigned int WIFI_NETWORKS_SCAN_MS = 4000;
  static constexpr unsigned int SENSOR_DETECTION_MS = 4000;
  static constexpr unsigned int DEBUG_FORCED_INITIALIZATION_DELAY = 20000;
};

namespace MqttEndpoints {
  static constexpr char AWS_IOT_CORE_ENDPOINT[] = "avo0w7o1tlck1-ats.iot.eu-west-1.amazonaws.com"; // TO DO: make IoT Core endpoint configurable at build time
  static constexpr char AWS_CERTIFICATES_PROVISIONING_TOPIC[] = "$aws/certificates/create/json";
  static constexpr char AWS_CERTIFICATES_PROVISIONING_RESPONSE_TOPIC[] = "$aws/certificates/create/json/accepted";
  static constexpr char AWS_DEVICE_PROVISIONING_TOPIC[] = "$aws/provisioning-templates/museum-alert-provisioning-template/provision/json";
  static constexpr char AWS_DEVICE_PROVISIONING_RESPONSE_TOPIC[] = "$aws/provisioning-templates/museum-alert-provisioning-template/provision/json/accepted";
  static constexpr char DEVICE_INCOMING_COMMANDS_TOPIC[] = "arn:aws:iot:eu-west-1:767398097786:topic/%s/sub"; // TO DO: make region and account id configurable at build time
  static constexpr char DEVICE_OUTGOING_DATA_TOPIC[] = "arn:aws:iot:eu-west-1:767398097786:topic/%s/pub";
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

  private:
    char ssid[SSID_SIZE];
    char password[PASSWORD_SIZE];

  public:
    WiFiCredentials() { clear(); }  // Constructor to initialize credentials

  bool isValid() const {
    return ssid[0] != '\0' && password[0] != '\0';
  }

  void clear() {
    ssid[0] = '\0';
    password[0] = '\0';
  }

  void setSSID(const char* newSSID) {
    if (newSSID) {
      strncpy(ssid, newSSID, SSID_SIZE - 1);
      ssid[SSID_SIZE - 1] = '\0';  // Ensure null-termination
    }
  }

  void setPassword(const char* newPassword) {
    if (newPassword) {
      strncpy(password, newPassword, PASSWORD_SIZE - 1);
      password[PASSWORD_SIZE - 1] = '\0';  // Ensure null-termination
    }
  }

  void getSSID(char* dest, size_t size) const {
    strncpy(dest, ssid, size - 1);
    dest[size - 1] = '\0';  // Ensure null-termination
  }

  void getPassword(char* dest, size_t size) const {
    strncpy(dest, password, size - 1);
    dest[size - 1] = '\0';  // Ensure null-termination
  }

};

struct Certificates {

  static constexpr size_t CERT_SIZE = 4096;
  static constexpr size_t KEY_SIZE = 4096;

  private:
    char clientCert[CERT_SIZE];
    char privateKey[KEY_SIZE];

  public:
    Certificates() { clear(); }  // Constructor to initialize certificates

  bool isValid() const {
    return clientCert[0] != '\0' && privateKey[0] != '\0';
  }

  void clear() {
    clientCert[0] = '\0';
    privateKey[0] = '\0';
  }

  void setClientCert(const char* cert) {
    if (cert) {
      strncpy(clientCert, cert, CERT_SIZE - 1);
      clientCert[CERT_SIZE - 1] = '\0';  // Ensure null-termination
    }
  }

  void setPrivateKey(const char* key) {
    if (key) {
      strncpy(privateKey, key, KEY_SIZE - 1);
      privateKey[KEY_SIZE - 1] = '\0';  // Ensure null-termination
    }
  }

  void getClientCert(char* dest, size_t size) const {
    strncpy(dest, clientCert, size - 1);
    dest[size - 1] = '\0';  // Ensure null-termination
  }

  void getPrivateKey(char* dest, size_t size) const {
    strncpy(dest, privateKey, size - 1);
    dest[size - 1] = '\0';  // Ensure null-termination
  }

};

struct ProvisioningPayload {

  static constexpr size_t TOKEN_SIZE = 256;
  static constexpr size_t THING_NAME_SIZE = 128;
  static constexpr size_t COMPANY_SIZE = 128;

  private:
    char certificateOwnershipToken[TOKEN_SIZE];
    char thingName[THING_NAME_SIZE];
    char company[COMPANY_SIZE];

  public:
    ProvisioningPayload() { clear(); }  // Constructor initializes the fields

  void clear() {
    certificateOwnershipToken[0] = '\0';
    thingName[0] = '\0';
    company[0] = '\0';
  }

  bool isValid() const {
    return certificateOwnershipToken[0] != '\0' &&
           thingName[0] != '\0' &&
           company[0] != '\0';
  }

  void setCertificateOwnershipToken(const char* token) {
    if (token) {
      strncpy(certificateOwnershipToken, token, TOKEN_SIZE - 1);
      certificateOwnershipToken[TOKEN_SIZE - 1] = '\0';
    }
  }

  void setThingName(const char* name) {
    if (name) {
      strncpy(thingName, name, THING_NAME_SIZE - 1);
      thingName[THING_NAME_SIZE - 1] = '\0';
    }
  }

  void setCompany(const char* comp) {
    if (comp) {
      strncpy(company, comp, COMPANY_SIZE - 1);
      company[COMPANY_SIZE - 1] = '\0';
    }
  }

  void getCertificateOwnershipToken(char* dest, size_t size) const {
    strncpy(dest, certificateOwnershipToken, size - 1);
    dest[size - 1] = '\0';
  }

  void getThingName(char* dest, size_t size) const {
    strncpy(dest, thingName, size - 1);
    dest[size - 1] = '\0';
  }

  void getCompany(char* dest, size_t size) const {
    strncpy(dest, company, size - 1);
    dest[size - 1] = '\0';
  }
};

struct ProvisioningSettings {

private:
  WiFiCredentials wiFiCredentials;
  Certificates certificates;

public:
  ProvisioningSettings() { clear(); }  // Constructor initializes the struct

  bool isValid() const {
    return wiFiCredentials.isValid() && certificates.isValid();
  }

  void clear() {
    wiFiCredentials.clear();
    certificates.clear();
  }

  void setWiFiCredentials(WiFiCredentials& credentials) {
    wiFiCredentials = credentials;
  }

  void setCertificates(Certificates& certificates) {
    certificates = certificates;
  }

  WiFiCredentials getWiFiCredentials() const {
    return wiFiCredentials;
  }

  Certificates getCertificates() const {
    return certificates;
  }

};

#endif

