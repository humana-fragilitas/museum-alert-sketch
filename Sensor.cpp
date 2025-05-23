#include "sensor.h"

unsigned long Sensor::durationMicroSec = 0;
unsigned long Sensor::distanceInCm = 0;
float Sensor::alarmDistance = DEFAULT_ALARM_DISTANCE;

char Sensor::name[32] = {0};  
char Sensor::incomingCommandsTopic[128] = {0};  
char Sensor::outgoingDataTopic[128] = {0};
String Sensor::clientCert = "";
String Sensor::privateKey = "";
String Sensor::companyName = "";

void Sensor::initialize() {

  uint64_t chipid = ESP.getEfuseMac();
  uint16_t chip = static_cast<std::uint16_t>(chipid >> 32);

  snprintf(Sensor::name, sizeof(Sensor::name), "MAS-%04X%08X", chip, static_cast<std::uint32_t>(chipid));

  DEBUG_PRINTF("Sensor name: %s\n", Sensor::name);

};

void Sensor::configure(DeviceConfiguration configuration) {

  DEBUG_PRINTLN("Configuring sensor client certificate, private key and MQTT topics...");
  
  Sensor::clientCert = configuration.certificates.clientCert;
  Sensor::privateKey = configuration.certificates.privateKey;
  Sensor::companyName = configuration.companyName;
  // TO DO: move this in a dedicated method
  // Sensor::alarmDistance = configuration.alarmDistance;

  snprintf(Sensor::outgoingDataTopic, sizeof(Sensor::outgoingDataTopic),
      MqttEndpoints::DEVICE_OUTGOING_DATA_TOPIC, Sensor::companyName.c_str(), Sensor::name);
  snprintf(Sensor::incomingCommandsTopic, sizeof(Sensor::incomingCommandsTopic),
      MqttEndpoints::DEVICE_INCOMING_COMMANDS_TOPIC, Sensor::companyName.c_str(), Sensor::name);

  DEBUG_PRINTLN("Configured sensor certificate and private key:");
  DEBUG_PRINTF("- client certificate: %s\n", Sensor::clientCert.c_str());
  DEBUG_PRINTF("- private key: %s\n", Sensor::privateKey.c_str());

  DEBUG_PRINTLN("Configured sensor MQTT topics:");
  DEBUG_PRINTF("- incoming messages topic: %s\n", Sensor::incomingCommandsTopic);
  DEBUG_PRINTF("- outgoing messages topic: %s\n", Sensor::outgoingDataTopic);

};

float Sensor::setDistance(float distance) {

  alarmDistance = (distance <= MAXIMUM_ALARM_DISTANCE) ?
    distance : MAXIMUM_ALARM_DISTANCE;

  DEBUG_PRINTF("Minimum alarm distance set to %f\n", alarmDistance);

  return alarmDistance;

};

bool Sensor::detect() {

  DEBUG_PRINTLN("Detecting distance...");

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
  if (m_hasAlarm = (distanceInCm < alarmDistance)) {

    DEBUG_PRINTF("Alarm! Distance detected: "
                  "%d cm\n", distanceInCm);

    JsonDocument alarmStatusPayload;
    alarmStatusPayload["distance"] = distanceInCm;

    return send(MqttMessageType::ALARM, alarmStatusPayload);

  }

  return true;

};

bool Sensor::send(MqttMessageType type, String correlationId, JsonVariant payload) {

  JsonDocument messageContent;

  messageContent["type"] = type;

  if (!payload.isNull()) {
    messageContent["data"] = payload;
  }

  if (correlationId.length()) {
    messageContent["cid"] = correlationId;
  }

  String messageContentString;
  serializeJson(messageContent, messageContentString);

  return (Sensor::isConnected) ? mqttClient.publish(
    Sensor::outgoingDataTopic,
    messageContentString.c_str()
  ) : false;

};

bool Sensor::send(MqttMessageType type, String correlationId) {

  return send(type, correlationId, JsonDocument().to<JsonVariant>());

};

bool Sensor::send(MqttMessageType type, JsonVariant payload) {

  return send(type, "", payload);

};

void Sensor::parseMqttCommand(String jsonPayload) {

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, jsonPayload);

  if (error) {
    DEBUG_PRINTLN("Failed to deserialize incoming sensor command json:");
    DEBUG_PRINTLN(error.c_str());
    return;
  }

  if (!doc["type"].is<MqttMessageType>() || !doc["cid"].is<String>()) {
    DEBUG_PRINTLN("Cannot process command payload with no type or correlation id; exiting...");
    return;
  }

  MqttCommandType commandType = doc["type"].as<MqttCommandType>();
  String correlationId = doc["cid"].as<String>();

  switch(commandType) {

    case MqttCommandType::RESET:
      DEBUG_PRINTLN("Received device reset command");
      onReset(correlationId);
      break;

    case MqttCommandType::GET_CONFIGURATION:
      DEBUG_PRINTLN("Received get configuration command");
      onGetConfiguration(correlationId);
      break;

    case MqttCommandType::SET_CONFIGURATION:
      DEBUG_PRINTLN("Received set configuration command");
      onSetConfiguration(doc, correlationId);
      break;

    default:
      DEBUG_PRINTF("Received unknown command with type: %d\n", commandType);

  }
  
};

bool Sensor::connect() {

  DEBUG_PRINTLN("Connecting sensor to MQTT broker...");

  bool success = mqttClient.connect(
    Sensor::clientCert.c_str(),
    Sensor::privateKey.c_str(),
    Sensor::name
  );

  mqttClient.subscribe(incomingCommandsTopic);

  return success;

};

bool Sensor::isConnected() {
  return mqttClient.isConnected();
};

bool Sensor::hasAlarm() {
  return m_hasAlarm;
};

bool Sensor::onReset(String correlationId) {

  // TO DO: why this conditional logic? Ack should be independent
  if(send(MqttMessageType::ACK, correlationId)) {
    DeviceControls::reset();
  }
  return false;

};

bool Sensor::onGetConfiguration(String correlationId) {

  JsonDocument configurationPayload;
  configurationPayload["distance"] = alarmDistance;
  configurationPayload["firmware"] = FIRMWARE_VERSION;
  return send(MqttMessageType::CONFIGURATION, correlationId, configurationPayload);

};

bool Sensor::onSetConfiguration(JsonVariant doc, String correlationId) {

  alarmDistance = doc["distance"].as<float>();

  if (alarmDistance >= MINIMUM_ALARM_DISTANCE &&
      alarmDistance <= MAXIMUM_ALARM_DISTANCE) {

    StorageManager::saveDistance(
      setDistance(alarmDistance)
    );

    return onGetConfiguration(correlationId);

  } else {

    DEBUG_PRINTF("Alarm distance is not within bounds: %f; cannot set configuration",
      alarmDistance
    );

    return false;

  }

};

bool Sensor::m_hasAlarm = false;

MQTTClient Sensor::mqttClient([](const char topic[], byte* payload, unsigned int length){

  String message = String((char*)payload).substring(0, length);

  DEBUG_PRINTF("Sensor received a command on topic '%s':\n", topic);
  DEBUG_PRINTLN(message);

  Sensor::parseMqttCommand(message);

});
