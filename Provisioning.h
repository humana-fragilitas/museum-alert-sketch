#include <Arduino.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <mqtt_client.h>

#include <functional>

#include "Macros.h"
#include "Sensor.h"
#include "Configuration.h"
#include "MQTTClient.h"
#include "CertManager.h"

#ifndef PROVISIONING
#define PROVISIONING

class Provisioning {

  private:
    MQTTClient mqttClient;
    CertManager certManager;
    ProvisioningPayload provisioningPayload;
    Certificates tempCertificates;
    std::function<void(bool)> m_onComplete;
    bool isRegistered = false;
    void onResponse(const char topic[], byte* payload, unsigned int length);
    void onCertificates(String message);
    void onDeviceRegistered(String message);

  public:
    Provisioning(std::function<void(bool)> onComplete);
    void registerDevice(Certificates certificates);

};

#endif