#ifndef SENSOR
#define SENSOR

class Sensor {

  private:
    float durationMicroSec, distanceincm;

  public:
    bool detect();

};

#endif