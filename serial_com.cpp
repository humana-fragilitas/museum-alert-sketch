#include "serial_com.h"

void initializeSerial() {

  unsigned const int waitTime = Timing::SERIAL_PORT_INIT_TIMEOUT_MS;
  unsigned long startTime = millis();

  Serial.begin(Communication::SERIAL_COM_BAUD_RATE);

  Serial.println("Initializing serial connection");

  while (!Serial) {
    if ((millis() - startTime) >= waitTime) {
      break;
    }
  }

  Serial.println(Serial ? "Serial port ready" : "Serial port unavailable: initialization timed out");

}