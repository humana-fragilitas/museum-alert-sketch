#ifndef SENSOR
#define SENSOR

#include<Arduino.h>
#include <ArduinoJson.h>
#include<esp_system.h>

#include "macros.h"
#include "pins.h"
#include "settings.h"
#include "mqtt_client.h"
#include "device_controls.h"

struct AlarmPayload {
  unsigned long detectedDistanceInCm;
  bool hasAlarm;
};

struct Commands {
  static constexpr int RESET = 1;
};

class Sensor {

  private:
    static MQTTClient mqttClient;
    static bool m_hasAlarm;
    static float minimumDistance;
    static constexpr float speedOfSoundPerMicrosec = 0.0343;
    static unsigned long durationMicroSec, distanceInCm;
    static String clientCert;
    static String privateKey;
    static String companyName;
    static char incomingCommandsTopic[128];
    static char outgoingDataTopic[128];
    static bool send(MqttMessageType type, JsonVariant payload);
    static void parseMqttCommand(String command);

  public:
    static char name[32];
    static void initialize();
    static bool connect();
    static bool detect();
    static void configure(DeviceConfiguration configuration);
    static bool isConnected();
    static bool hasAlarm();

};

#endif