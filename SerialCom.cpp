#include "SerialCom.h"

void initializeSerial() {

  #ifdef DEBUG

    unsigned const int waitTime = 2000;
    unsigned long startTime = millis();

    Serial.begin(9600);

    Serial.println("Initializing serial connection");

    while (!Serial) {

      if ((millis() - startTime) >= waitTime) {

        break;

      }

    }

    if (Serial) Serial.println("Serial port ready");

  #endif
 
}