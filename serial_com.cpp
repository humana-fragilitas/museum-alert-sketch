#include "serial_com.h"

void SerialCom::initialize(unsigned const int timeout) noexcept {

  if (Serial) return;

  const unsigned long startTime = millis();

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

void SerialCom::send(USBMessageType type, const String& cid, JsonVariant payload) {

  JsonDocument jsonPayload;
  String serializedJsonPayload;

  jsonPayload["type"] = static_cast<int>(type);
  jsonPayload["sn"] = Sensor::name;

  if (!cid.isEmpty()) {
    jsonPayload["cid"] = cid;
  }

  if (!payload.isNull()) {
    jsonPayload["data"] = payload;
  }

  serializeJson(jsonPayload, serializedJsonPayload);

  serializedJsonPayload = "<|" + serializedJsonPayload + "|>";

  if (Serial.availableForWrite() > 0) {
    Serial.println(serializedJsonPayload.c_str());
  } else {
    DEBUG_PRINTF("Serial port is unavailable for writing; skipping payload: %s\n", serializedJsonPayload.c_str());
  }

};

void SerialCom::error(ErrorType type, const String& correlationId) {

  JsonDocument jsonPayload;
  String serializedJsonPayload;

  jsonPayload["error"] = static_cast<int>(type);

  const String cid = !correlationId.isEmpty() ? correlationId : "";

  send(USBMessageType::ERROR, cid, jsonPayload);

};

void SerialCom::acknowledge(const String& correlationId) {

  send(USBMessageType::ACKNOWLEDGMENT, correlationId, JsonVariant());

};

String SerialCom::getStringWithMarkers() {

  char* receivedChars = new char[Communication::SERIAL_COM_BUFFER_SIZE];
  int ndx = 0;
  bool receiving = false;
  char rc;

  constexpr char startMarker[] = "<|";
  constexpr char endMarker[] = "|>";
  int startMatch = 0, endMatch = 0;

  while (true) {
    while (Serial.available() > 0) {
      rc = Serial.read();
      if (!receiving) {
        receiving = detectMarker(rc, startMarker, startMatch);
        if (receiving) {
          ndx = 0;   // Reset buffer index
          continue;  // Skip storing markers
        }
      } else {
        // Prevent overflow
        if (ndx < Communication::SERIAL_COM_BUFFER_SIZE - 1) {
          receivedChars[ndx++] = rc;
        }
        if (detectMarker(rc, endMarker, endMatch)) {
          // Remove "|>" and terminate
          receivedChars[ndx - 2] = '\0';
          String result = String(receivedChars);
          // Free heap memory
          delete[] receivedChars;
          return result;
        }
      }
    }
  }

};

RequestWrapper SerialCom::waitForRequest() {

  const auto jsonString = getStringWithMarkers();

  JsonDocument doc;
  RequestWrapper request{};

  const auto error = deserializeJson(doc, jsonString);
  if (error) {
    Serial.println("Failed to parse main JSON request");
    request.commandType = USBCommandType::USB_COMMAND_INVALID;
    return request;
  }

  request.correlationId = doc["cid"] | "";
  request.commandType = static_cast<USBCommandType>(doc["commandType"] | static_cast<int>(USBCommandType::USB_COMMAND_INVALID));

  // Extract payload as string (it might be an object or null)
  if (doc["payload"].is<JsonObject>()) {
    // If payload is an object, serialize it back to string
    String payloadStr;
    serializeJson(doc["payload"], payloadStr);
    request.payloadJson = payloadStr;
  } else if (doc["payload"].is<const char*>()) {
    // If payload is already a string
    request.payloadJson = doc["payload"].as<String>();
  } else {
    // If payload is null or other type
    request.payloadJson = "";
  }

  return request;

};

bool SerialCom::detectMarker(char rc, const char marker[], int& matchCount) noexcept {

  if (rc == marker[matchCount]) {
    ++matchCount;
    // Full marker matched
    if (marker[matchCount] == '\0') {
      matchCount = 0;
      return true;
    }
  } else {
    // Reset or recheck first char
    matchCount = (rc == marker[0]) ? 1 : 0;
  }
  return false;
  
};