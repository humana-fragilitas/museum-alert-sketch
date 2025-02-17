#include "serial_com.h"

/*
 THIS WORKS

 // Example 2 - Receive with an end-marker 

const int numChars = 4096; // set buffer properly
char receivedChars[numChars];   // an array to store the received data

boolean newData = false;

void setup() {
    Serial.begin(9600);
    Serial.println("<Arduino is ready>");
}

void loop() {
    recvWithEndMarker();
    showNewData();
}

void recvWithEndMarker() {
    static int ndx = 0;
    char endMarker = '\n';
    char rc;
    
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (rc != endMarker) {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= numChars) {
                ndx = numChars - 1;
            }
        }
        else {
            receivedChars[ndx] = '\0'; // terminate the string
            ndx = 0;
            newData = true;
        }
    }
}

void showNewData() {
    if (newData == true) {
        Serial.print("This just in ... ");
        Serial.println(receivedChars);
        newData = false;
    }
}


*/

void SerialCom::initialize(unsigned const int timeout) {

  if (Serial) return;

  unsigned long startTime = millis();

  Serial.begin(Communication::SERIAL_COM_BAUD_RATE);

  Serial.setTimeout(10000);

  Serial.println("Initializing serial connection");

  while (!Serial) {
    if ((millis() - startTime) >= timeout) {
      break;
    }
  }

  Serial.println(Serial ? "Serial port ready" : "Serial port unavailable: initialization timed out");
  
};

void SerialCom::send(String payload) {

  if (Serial.availableForWrite() > 0) {
    Serial.println(payload.c_str());
  } else {
    DEBUG_PRINTF("Serial port is unavailable for writing; skipping payload: %s\n", payload.c_str());
  }

}

String SerialCom::receiveProvisiongSettings() {

  String message;

  while (Serial.available() > 0) {
    message = Serial.readStringUntil('|');
  }

  if (!message.isEmpty()) {
    DEBUG_PRINTF("Received data via serial communication: %s\n", message.c_str());
    return message;
  }

  return "";

}