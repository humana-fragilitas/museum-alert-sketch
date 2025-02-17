#include "serial_com.h"

// void initializeSerial() {

//   unsigned const int waitTime = Timing::SERIAL_PORT_INIT_TIMEOUT_MS; // questo deve essere il default del metodo invocato
//   unsigned long startTime = millis();

//   Serial.begin(Communication::SERIAL_COM_BAUD_RATE);

//   Serial.println("Initializing serial connection");

//   while (!Serial) {
//     if ((millis() - startTime) >= waitTime) {
//       break;
//     }
//   }

//   Serial.println(Serial ? "Serial port ready" : "Serial port unavailable: initialization timed out");

// }

void SerialCom::initialize(unsigned const int timeout) {

  unsigned long startTime = millis();

  Serial.begin(Communication::SERIAL_COM_BAUD_RATE);

  Serial.println("Initializing serial connection");

  while (!Serial) {
    if ((millis() - startTime) >= timeout) {
      break;
    }
  }

  Serial.println(Serial ? "Serial port ready" : "Serial port unavailable: initialization timed out");
  
};

void SerialCom::send(String payload) {

}

ProvisioningSettings SerialCom::receiveProvisiongSettings() {

  ProvisioningSettings provisiongSettings;

  return provisiongSettings;


}