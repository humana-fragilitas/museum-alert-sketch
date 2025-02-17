#include "sensor.h"

unsigned long Sensor::durationMicroSec = 0;
unsigned long Sensor::distanceInCm = 0;

char Sensor::name[32] = {0};  
char Sensor::incomingCommandsTopic[128] = {0};  
char Sensor::outgoingDataTopic[128] = {0};  

void Sensor::initialize() {

  createName();
  snprintf(Sensor::outgoingDataTopic, sizeof(Sensor::outgoingDataTopic), MqttEndpoints::DEVICE_OUTGOING_DATA_TOPIC, Sensor::name);
  snprintf(Sensor::incomingCommandsTopic, sizeof(Sensor::incomingCommandsTopic), MqttEndpoints::DEVICE_INCOMING_COMMANDS_TOPIC, Sensor::name);

};

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

  char alarmStatusPayloadString[128];
  serializeJson(alarmStatusPayload, alarmStatusPayloadString);

  return mqttClient.publish(
    Sensor::outgoingDataTopic,
    alarmStatusPayloadString
  );

};

void Sensor::createName() {

  uint64_t chipid = ESP.getEfuseMac();
  uint16_t chip = static_cast<std::uint16_t>(chipid >> 32);

  snprintf(Sensor::name, sizeof(Sensor::name), "MAS-%04X%08X", chip, static_cast<std::uint32_t>(chipid));

  DEBUG_PRINTF("Sensor name: %s\n", Sensor::name);

};

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
    certificates.clientCert,
    certificates.privateKey,
    Sensor::name
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
