#ifndef SENSOR
#define SENSOR

#include<Arduino.h>
#include <ArduinoJson.h>
#include<esp_system.h>

#include "macros.h"
#include "pins.h"
#include "settings.h"
#include "mqtt_client.h"

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
    static constexpr float minimumDistance = 10.0;
    static constexpr float speedOfSoundPerMicrosec = 0.0343;
    static unsigned long durationMicroSec, distanceInCm;
    static char incomingCommandsTopic[128];
    static char outgoingDataTopic[128];
    static void createName(char* nameBuffer);
    static void parseMqttCommand(String command);

  public:
    static char name[32];
    static void initialize();
    static bool connect(Certificates certificates);
    static AlarmPayload detect();
    static bool report(AlarmPayload payload);
    static bool isConnected();

};

#endif