#include<Arduino.h>
#include<esp_system.h>

#include "Pins.h"
#include "Configuration.h"
#include "MQTTClient.h"

#ifndef SENSOR
#define SENSOR

class Sensor {

  private:
    static MQTTClient mqttClient;
    static constexpr float minimumDistance = 10.0;
    static constexpr float speedOfSoundPerMicrosec = 0.0343;
    static unsigned long durationMicroSec, distanceInCm;
    static String createName();
    static void parseMqttCommand(String command);

  public:
    static const String sensorName;
    static bool connect(Certificates certificates);
    static bool detect();

};

#endif