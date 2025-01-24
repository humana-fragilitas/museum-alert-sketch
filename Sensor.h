#ifndef SENSOR
#define SENSOR

class Sensor {

  private:
    static constexpr float minimumDistance = 10.0;
    static constexpr float speedOfSoundPerMicrosec = 0.0343;
    static unsigned long durationMicroSec, distanceInCm;
    static String createName();

  public:
    static const String sensorName;
    static bool detect();

};

#endif