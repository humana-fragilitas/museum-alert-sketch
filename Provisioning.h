#ifndef PROVISIONING
#define PROVISIONING

#include <Arduino.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <mqtt_client.h>

#include <functional>

#include "macros.h"
#include "sensor.h"
#include "settings.h"
#include "mqtt_client.h"
#include "cert_manager.h"

class Provisioning {

  private:
    MQTTClient mqttClient;
    CertManager certManager;
    Certificates tempCertificates;
    std::function<void(bool)> m_onComplete;
    bool isRegistered = false;
    void onResponse(const char topic[], byte* payload, unsigned int length);
    void onDeviceRegistered(const char* message);
    void onCertificates(const char* message);

  public:
    Provisioning(std::function<void(bool)> onComplete);
    static ProvisioningSettings parse(String settingsJson);
    static WiFiCredentials parseWiFiCredentialsJSON(String wiFiCredentialsJson);
    void registerDevice(Certificates certificates);

};

#endif