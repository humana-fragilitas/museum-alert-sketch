#include "sensor.h"

unsigned long Sensor::durationMicroSec = 0;
unsigned long Sensor::distanceInCm = 0;
float Sensor::minimumDistance = DEFAULT_ALARM_DISTANCE;

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
  Sensor:companyName = configuration.companyName;
  Sensor::minimumDistance = configuration.alarmDistance;

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

bool Sensor::detect() {

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
  if (hasAlarm = (distanceInCm > minimumDistance)) {

    DEBUG_PRINTF("Alarm! Distance detected: "
                  "%d cm\n", distanceInCm);

  }

  m_hasAlarm = hasAlarm;

  JsonDocument alarmStatusPayload;
  alarmStatusPayload["hasAlarm"] = hasAlarm;
  alarmStatusPayload["distance"] = distanceInCm;

  // TO DO: alarms should only be broadcasted if there is
  // an actual distance breach
  return send(MqttMessageType::ALARM, alarmStatusPayload);

};

bool Sensor::send(MqttMessageType type, JsonVariant payload) {

  char alarmStatusPayloadString[128];
  payload["type"] = type;
  serializeJson(payload, alarmStatusPayloadString);

  return mqttClient.publish(
    Sensor::outgoingDataTopic,
    alarmStatusPayloadString
  );

};

void Sensor::parseMqttCommand(String jsonPayload) {

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, jsonPayload);

  if (error) {
    DEBUG_PRINTLN("Failed to deserialize incoming sensor command json:");
    DEBUG_PRINTLN(error.c_str());
    return;
  }

  MqttCommandType commandType = doc["type"].as<MqttCommandType>();

  switch(commandType) {

    case MqttCommandType::RESET:
      DEBUG_PRINTLN("Received device reset command");
      DeviceControls::reset();
      break;
    case MqttCommandType::GET_CONFIGURATION:
      DEBUG_PRINTLN("Received get configuration command");
      break;
    case MqttCommandType::SET_CONFIGURATION:
      DEBUG_PRINTLN("Received set configuration command");
      // TO DO: persist minimum distance
      minimumDistance =  doc["distance"].as<float>();
      DEBUG_PRINTF("Setting minimum alarm distance to %f\n", minimumDistance);
      break;
    default:
      DEBUG_PRINTF("Received unknown command with id %d\n", commandType);

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

bool Sensor::m_hasAlarm = false;

MQTTClient Sensor::mqttClient([](const char topic[], byte* payload, unsigned int length){

  String message = String((char*)payload).substring(0, length);

  DEBUG_PRINTF("Sensor received a command on topic '%s':\n", topic);
  DEBUG_PRINTLN(message);

  Sensor::parseMqttCommand(message);

});
