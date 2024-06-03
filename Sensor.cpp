#include<Arduino.h>

#include "Pins.h"
#include "Sensor.h"

#define MINIMUM_DISTANCE 10.0

bool Sensor::detect() {

  bool hasAlarm = false;

  /* Send 10 microsec pulse to TRIG pin*/
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  /* measure pulse duration from ECHO pin*/
  durationMicroSec = pulseIn(echoPin, HIGH);
  /* calculate distance*/
  distanceincm = 0.017 * durationMicroSec;
  /*Display distance on Serial Monitor*/
  //Serial.print("distance: ");
  //Serial.print(distanceincm);  /*Print distance in cm*/
  //Serial.println(" cm");

  if (hasAlarm = (distanceincm < MINIMUM_DISTANCE)) {

    digitalWrite(alarmPin, HIGH);
    Serial.print("\nAlarm! Distance detected: ");
    Serial.print(distanceincm);
    Serial.print(" cm");

  } else {

    digitalWrite(alarmPin, LOW);

  }

  return hasAlarm;

}