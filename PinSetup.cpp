#include<Arduino.h>

#include "Pins.h"

void pinSetup() {

  pinMode(triggerPin, OUTPUT);
  pinMode(alarmPin, OUTPUT);
  pinMode(wiFiPin, OUTPUT);
  pinMode(appStatusPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(resetButtonPin, INPUT);

  digitalWrite(triggerPin, LOW);
  digitalWrite(alarmPin, LOW);
  digitalWrite(wiFiPin, LOW);
  digitalWrite(echoPin, LOW);

}

