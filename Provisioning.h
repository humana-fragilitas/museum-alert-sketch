#include <Arduino.h>
#include <Preferences.h>
#include <mqtt_client.h>

#include <functional>

#include "Configuration.h"
#include "MQTTClient.h"

#ifndef PROVISIONING
#define PROVISIONING

class Provisioning {

  private:
    MQTTClient mqttClient;
    bool isRegistered;
    void onCertificates(const char topic[], byte* payload, unsigned int length);

  public:
    Provisioning();
    bool addDevice(ProvisioningPayload payload);

};

#endif

