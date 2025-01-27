#include<Arduino.h>
#include<esp_system.h>

#include "Pins.h"
#include "Sensor.h"

// #define MINIMUM_DISTANCE 10.0
// #define SPEED_OF_SOUND_CM_MICROSEC 0.0343

// Initialize static members outside the class
unsigned long Sensor::durationMicroSec = 0;
unsigned long Sensor::distanceInCm = 0;

/**
 * In C++ 17 this could have been written inline:
 * static const std::string sensorName = createName();
 */
const String Sensor::sensorName = Sensor::createName();

bool Sensor::detect() {

  bool hasAlarm = false;

  /* Send a 10 microseconds pulse to TRIG pin*/
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  
  /* Measure pulse duration from ECHO pin in microseconds */
  durationMicroSec = pulseIn(echoPin, HIGH);

  /* Convert the round-trip time (measured in microseconds)
     into a one-way distance in centimeters  */
  distanceInCm = (speedOfSoundPerMicrosec / 2) * durationMicroSec;

  // Note: assignment and evaluation
  if (hasAlarm = (distanceInCm < minimumDistance)) {

    digitalWrite(alarmPin, HIGH);
    Serial.print("\nAlarm! Distance detected: ");
    Serial.print(distanceInCm);
    Serial.print(" cm");

  } else {

    digitalWrite(alarmPin, LOW);

  }

  return hasAlarm;

}

String Sensor::createName() {

  std::array<char, 33> sensorName = {};

  // Get the chip ID for ESP8266
  auto chipid = ESP.getEfuseMac();  // Use ESP.getChipId() for ESP8266
  auto chip = static_cast<std::uint16_t>(chipid >> 32);

  std::snprintf(sensorName.data(),
                sensorName.size(),
                "MAS-%04X%08X",
                chip,
                static_cast<std::uint32_t>(chipid));

  return String(sensorName.data());

}


