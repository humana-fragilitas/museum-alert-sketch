#ifndef SENSOR
#define SENSOR

#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp_system.h>

#include "config.h"
#include "mqtt_client.h"
#include "device_controls.h"
#include "storage_manager.h"
#include "ble_manager.h"


class Sensor {

  private:
    static MQTTClient mqttClient;
    static bool hasAlarm;
    static float alarmDistance;
    static String broadcastUrl;
    static constexpr float speedOfSoundPerMicrosec = 0.0343;
    static unsigned long durationMicroSec, distanceInCm;
    static String clientCert;
    static String privateKey;
    static String companyName;
    static char incomingCommandsTopic[128];
    static char outgoingDataTopic[128];
    static bool send(MqttMessageType type, const String& correlationId, const JsonVariant& payload);
    static bool send(MqttMessageType type, const String& correlationId);
    static bool send(MqttMessageType type, const JsonVariant& payload);
    static void parseMqttCommand(const String& command);

  public:
    static char name[32];
    static void initialize();
    static bool connect();
    static bool detect();
    static void configure(const AwsIotConfiguration& configuration);
    static float setDistance(float distance);
    static bool isConnected();
    static bool isAlarmActive();
    static bool isValidCommand(MqttCommandType type);
    static bool onReset(const String& correlationId);
    static bool onGetConfiguration(const String& correlationId);
    static bool onSetConfiguration(const JsonVariant& doc, const String& correlationId);
    static String setBroadcastUrl(const String& url);

};

#endif