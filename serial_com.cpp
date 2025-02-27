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

void SerialCom::send(MessageType type, JsonVariant payload) {

  JsonDocument jsonPayload;
  String serializedJsonPayload;

  jsonPayload["type"] = type;
  jsonPayload["data"] = payload;

  serializeJson(jsonPayload, serializedJsonPayload);

  serializedJsonPayload = "<|" + serializedJsonPayload + "|>";

  if (Serial.availableForWrite() > 0) {
    Serial.println(serializedJsonPayload.c_str());
  } else {
    DEBUG_PRINTF("Serial port is unavailable for writing; skipping payload: %s\n", serializedJsonPayload.c_str());
  }

}

String SerialCom::getStringWithMarkers() {

  constexpr int BUFFER_SIZE = 4096;
  char receivedChars[BUFFER_SIZE];
  int ndx = 0;
  bool receiving = false;
  char rc;

  const char startMarker[] = "<|";
  const char endMarker[] = "|>";
  int startMatch = 0, endMatch = 0;

  while (true) {
      while (Serial.available() > 0) {
          rc = Serial.read();

          if (!receiving) {
              receiving = detectMarker(rc, startMarker, startMatch);
              if (receiving) {
                  ndx = 0;  // Reset buffer index
                  continue;  // Skip storing markers
              }
          } else {
              if (ndx < BUFFER_SIZE - 1) {  // Prevent overflow
                  receivedChars[ndx++] = rc;
              }

              if (detectMarker(rc, endMarker, endMatch)) {
                  receivedChars[ndx - 2] = '\0';  // Remove "|>" and terminate
                  return String(receivedChars);
              }
          }
      }
  }

};

bool SerialCom::detectMarker(char rc, const char marker[], int &matchCount) {
      if (rc == marker[matchCount]) {
          matchCount++;
          if (marker[matchCount] == '\0') {  // Full marker matched
              matchCount = 0;
              return true;
          }
      } else {
          matchCount = (rc == marker[0]) ? 1 : 0;  // Reset or recheck first char
      }
      return false;
  };