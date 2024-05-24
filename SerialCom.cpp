#include<Arduino.h>

void initializeSerial() {

  unsigned const int serialReadinessWaitTime = 2000;
  unsigned int lastWaitTime = 0;

  Serial.begin(9600);

  Serial.println("Initializing serial connection");

  while (!Serial) {

    unsigned long currentMillis = millis();

    if ((currentMillis - lastWaitTime) >= serialReadinessWaitTime) {

      break;

    }

  }

  if (Serial) Serial.println("Serial port ready");
 
}