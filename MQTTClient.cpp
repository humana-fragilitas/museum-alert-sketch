#include "MQTTClient.h"

MQTTClient::MQTTClient(std::function<void(const char[], byte*, unsigned int)> onMqttEvent) :
    m_onMqttEvent{onMqttEvent}, net{}, client{net}, subscribedTopics{} {

      instanceCount++;
      String taskName = "MQTT_CLIENT_LOOP_TASK_" + String(instanceCount);
      xTaskCreate(&MQTTClient::loopTask, taskName.c_str(), 4096, this, 1, &loopTaskHandle);

    }

MQTTClient::~MQTTClient() {

  DEBUG_PRINTLN("MQTTClient Destructor: unsubscribing from topics, "
                  "disconnecting and deleting tasks...");

  for (const String& topic : subscribedTopics) {
    client.unsubscribe(topic.c_str());
    DEBUG_PRINTF("Mqtt client unsubscribing from topic: %s\n", topic.c_str());
  }
  subscribedTopics.clear();
    
  client.disconnect();
  vTaskDelete(loopTaskHandle);
  instanceCount--;

};

bool MQTTClient::connect(const char certPem[], const char privateKey[], const char clientId[]) {

  DEBUG_PRINTLN("Configuring MQTT client instance");

  net.setCACert(AWS_CERT_CA);
  net.setCertificate(certPem);
  net.setPrivateKey(privateKey);

  client.setServer(MqttEndpoints::AWS_IOT_CORE_ENDPOINT.c_str(), 8883);
  client.setCallback(m_onMqttEvent);

  DEBUG_PRINTF("Mqtt client connecting to AWS IoT Core endpoint with id: %s\n", clientId);

  while (!client.connect(clientId)) {
    DEBUG_PRINT(client.state());
    DEBUG_PRINT(".");
    delay(100);
  }

  if (!client.connected()) {
    DEBUG_PRINTLN("Mqtt client timed out while connecting to AWS IoT Core endpoint. Exiting...");
    return false;
  }

  DEBUG_PRINTLN("AWS IoT Core connected!");

  return true;

};

bool MQTTClient::publish(const char topic[], const char json[]) {

  // return client.publish("MAS-EC357A188534/pub", json);
  return client.publish(topic, json);
  
};

void MQTTClient::subscribe(const String& topic) {

  auto result = subscribedTopics.insert(topic);

  if (result.second) { 
    if (client.subscribe(topic.c_str())) {
      DEBUG_PRINTF("Mqtt client subscribed to topic: %s", topic);
    } else {
      DEBUG_PRINTF("Mqtt client failed to subscribe to topic: %s", topic);
      subscribedTopics.erase(topic);
    }
  } else {
    DEBUG_PRINTF("Mqtt client failed to subscribe to topic: %s", topic);
  }

};

void MQTTClient::loopTask(void *pvParameters) {

  MQTTClient *instance = static_cast<MQTTClient*>(pvParameters);
  if (instance->client.connected()) instance->client.loop();
  vTaskDelay(pdMS_TO_TICKS(1000));

};

bool MQTTClient::isConnected() {
  return client.connected();
}

int MQTTClient::instanceCount = 0;