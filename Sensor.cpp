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
const String Sensor::name = Sensor::createName();
const String Sensor::outgoingDataTopic = Sensor::getOutgoingDataTopic();
const String Sensor::incomingCommandsTopic = Sensor::getIncomingCommandsTopic();

AlarmPayload Sensor::detect() {

  DEBUG_PRINTLN("Detecting distance...");

  bool hasAlarm = false;

  /* Send a 10 microseconds pulse to TRIG pin*/
  digitalWrite(Pins::Trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(Pins::Trigger, LOW);
  
  /* Measure pulse duration from ECHO pin in microseconds */
  durationMicroSec = pulseIn(Pins::Echo, HIGH);

  /* Convert the round-trip time (measured in microseconds)
     into a one-way distance in centimeters  */
  distanceInCm = (speedOfSoundPerMicrosec / 2) * durationMicroSec;

  // Note: assignment and evaluation
  if (hasAlarm = (distanceInCm < minimumDistance)) {

    DEBUG_PRINTF("Alarm! Distance detected: "
                  "%d cm\n", distanceInCm);

  }

  AlarmPayload alarmPayload;
  alarmPayload.hasAlarm = hasAlarm;
  alarmPayload.detectedDistanceInCm = distanceInCm;

  return alarmPayload;

};

bool Sensor::report(AlarmPayload payload) {

  JsonDocument alarmStatusPayload;
  alarmStatusPayload["hasAlarm"] = payload.hasAlarm;
  alarmStatusPayload["distance"] = payload.detectedDistanceInCm;

  String alarmStatusPayloadString;
  serializeJson(alarmStatusPayload, alarmStatusPayloadString);

  return mqttClient.publish(
    Sensor::outgoingDataTopic.c_str(),
    alarmStatusPayloadString.c_str()
  );

};

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

};

String Sensor::getOutgoingDataTopic() {

  char buffer[50]; // TO DO: ensure buffer is large enough
  snprintf(buffer, sizeof(buffer), MqttEndpoints::DEVICE_OUTGOING_DATA_TOPIC.c_str(), Sensor::name.c_str());
  return String(buffer);

};

String Sensor::getIncomingCommandsTopic() {

  char buffer[50]; // TO DO: ensure buffer is large enough
  snprintf(buffer, sizeof(buffer), MqttEndpoints::DEVICE_INCOMING_COMMANDS_TOPIC.c_str(), Sensor::name.c_str());
  return String(buffer);

}

void Sensor::parseMqttCommand(String command) {

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, command);

  if (error) {
    DEBUG_PRINTLN("Failed to deserialize incoming sensor command json:");
    DEBUG_PRINTLN(error.c_str());
    return;
  }

  unsigned const int commandId = doc["id"].as<int>();

  switch(commandId) {

    case 1:
      DEBUG_PRINTLN("Received device reset command");
      break;
    default:
      DEBUG_PRINTF("Received unknown command with id %d", commandId);

  }

};

bool Sensor::connect(Certificates certificates) {

  DEBUG_PRINTLN("Connecting sensor to MQTT broker...");

  mqttClient.subscribe(incomingCommandsTopic);

  return mqttClient.connect(
    certificates.clientCert.c_str(),
    certificates.privateKey.c_str(),
    Sensor::name.c_str()
  );

};

MQTTClient Sensor::mqttClient([](const char topic[], byte* payload, unsigned int length){

  String message = String((char*)payload).substring(0, length);

  DEBUG_PRINTF("Sensor received a command on topic '%s':\n", topic);
  DEBUG_PRINTLN(message);

  Sensor::parseMqttCommand(message);

});

bool Sensor::isConnected() {
  return mqttClient.isConnected();
}
