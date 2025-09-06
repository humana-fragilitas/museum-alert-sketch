#ifndef PROVISIONING
#define PROVISIONING


#include <Arduino.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <mqtt_client.h>

#include <functional>

#include "sensor.h"
#include "config.h"
#include "mqtt_client.h"


class Provisioning {

  private:
    MQTTClient mqttClient;
    Certificates tempCertificates;
    AwsIotConfiguration configuration;
    String idToken;
    std::function<void(bool, const AwsIotConfiguration&)> m_onComplete;
    bool isRegistered{ false };
    void onResponse(const char topic[], byte* payload, unsigned int length);
    void onDeviceRegistered(const char* message);
    void onCertificates(const char* message, bool success = false);

  public:
    Provisioning(std::function<void(bool, const AwsIotConfiguration&)> onComplete);
    void registerDevice(const Certificates& certificates);
    
};

#endif