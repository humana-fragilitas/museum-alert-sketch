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

class Provisioning {

  private:
    MQTTClient mqttClient;
    Certificates tempCertificates;
    DeviceConfiguration configuration;
    String idToken;
    std::function<void(bool, DeviceConfiguration)> m_onComplete;
    bool isRegistered = false;
    void onResponse(const char topic[], byte* payload, unsigned int length);
    void onDeviceRegistered(const char* message);
    void onCertificates(const char* message, bool success = false);

  public:
    Provisioning(std::function<void(bool, DeviceConfiguration)> onComplete);
    static Certificates parseProvisioningCertificates(String settingsJson);
    static WiFiCredentials parseWiFiCredentialsJSON(String wiFiCredentialsJson);
    void registerDevice(Certificates certificates);

};

#endif