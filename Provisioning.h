#include <Arduino.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <mqtt_client.h>

#include <functional>

#include "Configuration.h"
#include "MQTTClient.h"

#ifndef PROVISIONING
#define PROVISIONING

class Provisioning {

  private:
    MQTTClient mqttClient;
    bool isRegistered = false;
    void onCertificates(const char topic[], byte* payload, unsigned int length);
    void onResponse(const char topic[], byte* payload, unsigned int length);
    void onCertificates(byte* payload, unsigned int length);
    void onDeviceRegistered(byte* payload, unsigned int length);

  public:
    Provisioning();
    void registerDevice(Certificates certificates);

};

#endif

