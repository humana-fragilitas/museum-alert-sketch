#include "serial_com.h"

USBCDC SerialCom::initializeSerial(unsigned const int waitTime) {

  unsigned long startTime = millis();

  Serial.begin(Communication::SERIAL_COM_BAUD_RATE);

  Serial.println("Initializing serial connection");

  while (!Serial) {
    if ((millis() - startTime) >= waitTime) {
      break;
    }
  }

  Serial.println(Serial ? "Serial port ready" : "Serial port unavailable: initialization timed out");

  return Serial;

};

void SerialCom::sendAvailableWiFiNetworks(String wifiScanJson) {

  Serial.println(wifiScanJson);

};

ProvisioningSettings SerialCom::getProvisioningSettings() {

  ProvisioningSettings provisioningSettings;

  String jsonString = Serial.readStringUntil('\n');

  DEBUG_PRINTF("Received provisioning settings JSON: %s\n", jsonString.c_str());

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, jsonString);

  if (error) {
      DEBUG_PRINTF("Provisioning settings JSON parsing failed: %s", error.c_str());
      return provisioningSettings;
  }

  const char *ssid = doc["ssid"];
  const char *password = doc["pass"];
  const char *tempCertPem = doc["tempCertPem"];
  const char *tempPrivateKey = doc["tempPrivateKey"];

  WiFiCredentials wiFiCredentials;
  snprintf(wiFiCredentials.ssid, WiFiCredentials::SSID_SIZE, ssid);
  snprintf(wiFiCredentials.password, WiFiCredentials::PASSWORD_SIZE, password);

  Certificates certificates;
  snprintf(certificates.clientCert, Certificates::CERT_SIZE, tempCertPem);
  snprintf(certificates.privateKey, Certificates::KEY_SIZE, tempPrivateKey);

  provisioningSettings.wiFiCredentials = wiFiCredentials;
  provisioningSettings.certificates = certificates;

  return provisioningSettings;

};