#ifndef MQTT_CLIENT
#define MQTT_CLIENT

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include <set>
#include <array>

#include "config.h"
#include "wifi_manager.h"

class MQTTClient {

  private:
    WiFiClientSecure net;
    PubSubClient client;
    String m_clientId;
    String m_certPem;
    String m_privateKey;
    bool hasAttemptedConnection = false;
    std::function<void(const char[], byte*, unsigned int)> m_onMqttEvent;
    std::set<String> subscribedTopics;
    static int instanceCount;
    void loopTask();
    static void loopTaskWrapper(void* pvParameters);
    TaskHandle_t loopTaskHandle = nullptr;
    static const char AWS_CERT_CA[];

  public:
    MQTTClient(std::function<void(const char[], byte*, unsigned int)> onMqttEvent);
    ~MQTTClient();
    bool connect(String certPem, String privateKey, String clientId = "MUSEUM-ALERT-CLIENT");
    bool publish(const char topic[], const char json[]);
    bool isConnected();
    void subscribe(const char topic[]);
    void loop();

};

#endif