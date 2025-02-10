#include "MQTTClient.h"

int MQTTClient::instanceCount = 0;

MQTTClient::MQTTClient(std::function<void(const char[], byte*, unsigned int)> onMqttEvent)
    : m_onMqttEvent{onMqttEvent}, net{}, client{net} {

    instanceCount++;
    String taskName = "MQTT_CLIENT_LOOP_TASK_" + String(instanceCount);
    xTaskCreate(&MQTTClient::loopTask, taskName.c_str(), 4096, this, 1, &loopTaskHandle);
    
}

MQTTClient::~MQTTClient() {
  
  DEBUG_PRINTLN("MQTTClient Destructor: Unsubscribing, disconnecting, and deleting task...");

  for (const auto& topic : subscribedTopics) {
      client.unsubscribe(topic.data());
      DEBUG_PRINTF("Unsubscribing from topic: %s\n", topic.data());
  }
  subscribedTopics.clear();
  
  client.disconnect();

  if (loopTaskHandle) {
      vTaskDelete(loopTaskHandle);
      loopTaskHandle = nullptr;
  }

  instanceCount--;

}

bool MQTTClient::connect(const char certPem[], const char privateKey[], const char clientId[]) {

  if (!certPem || !privateKey || !clientId) {
      DEBUG_PRINTLN("MQTTClient::connect: Invalid parameters!");
      return false;
  }

  DEBUG_PRINTLN("Configuring MQTT client instance");

  net.setCACert(AWS_CERT_CA);
  net.setCertificate(certPem);
  net.setPrivateKey(privateKey);

  client.setServer(MqttEndpoints::AWS_IOT_CORE_ENDPOINT, 8883);
  client.setCallback(m_onMqttEvent);

  DEBUG_PRINTF("Connecting to AWS IoT Core endpoint with ID: %s\n", clientId);

  int attempts = 0;
  while (!client.connect(clientId) && attempts < 10) {
      DEBUG_PRINT(client.state());
      DEBUG_PRINT(".");
      delay(500);
      attempts++;
  }

  if (!client.connected()) {
      DEBUG_PRINTLN("MQTTClient: Connection to AWS IoT Core timed out.");
      return false;
  }

  DEBUG_PRINTLN("AWS IoT Core connected!");
  return true;

}

bool MQTTClient::publish(const char topic[], const char json[]) {

  if (!topic || !json) {
      DEBUG_PRINTLN("MQTTClient::publish: Null topic or message!");
      return false;
  }
  return client.publish(topic, json);

}

void MQTTClient::subscribe(const char topic[]) {

  if (!topic || topic[0] == '\0') {
      DEBUG_PRINTLN("MQTTClient::subscribe: Invalid topic!");
      return;
  }

  std::array<char, 128> topicBuffer{};
  strncpy(topicBuffer.data(), topic, topicBuffer.size() - 1);

  auto result = subscribedTopics.insert(topicBuffer);

  if (result.second) { 
      if (client.subscribe(topic)) {
          DEBUG_PRINTF("Subscribed to topic: %s\n", topic);
      } else {
          DEBUG_PRINTF("Failed to subscribe to topic: %s\n", topic);
          subscribedTopics.erase(topicBuffer);
      }
  } else {
      DEBUG_PRINTF("Already subscribed to topic: %s\n", topic);
  }

}

void MQTTClient::loopTask(void *pvParameters) {
    MQTTClient *instance = static_cast<MQTTClient*>(pvParameters);
    while (instance->client.connected()) {
        instance->client.loop();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

bool MQTTClient::isConnected() {
    return client.connected();
}

