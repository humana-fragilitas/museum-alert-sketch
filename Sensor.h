#include<Arduino.h>
#include <ArduinoJson.h>
#include<esp_system.h>

#include "macros.h"
#include "Pins.h"
#include "Configuration.h"
#include "MQTTClient.h"

#ifndef SENSOR
#define SENSOR

struct AlarmPayload {
  unsigned long detectedDistanceInCm;
  bool hasAlarm;
};

class Sensor {

  private:
    static MQTTClient mqttClient;
    static constexpr float minimumDistance = 10.0;
    static constexpr float speedOfSoundPerMicrosec = 0.0343;
    static unsigned long durationMicroSec, distanceInCm;
    static String createName();
    static String getOutgoingDataTopic();
    static String getIncomingDataTopic();
    static const String incomingDataTopic;
    static const String outgoingDataTopic;
    static void parseMqttCommand(String command);

  public:
    static const String name;
    static bool connect(Certificates certificates);
    static AlarmPayload detect();
    static bool report(AlarmPayload payload);
    static bool isConnected();

};

#endif