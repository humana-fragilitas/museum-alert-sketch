#include "MQTTClient.h"

MQTTClient::MQTTClient(std::function<void(const char[], byte*, unsigned int)> onMqttEvent) :
    m_onMqttEvent{onMqttEvent}, net{}, client{net} {

      instanceCount++;
      String taskName = "MQTT_CLIENT_LOOP_TASK_" + String(instanceCount);
      xTaskCreate(&MQTTClient::loopTask, taskName.c_str(), 4096, NULL, 1, &loopTaskHandle);

    }

MQTTClient::~MQTTClient() {

    DEBUG_PRINTLN("MQTTClient Destructor: unsubscribing from topics, "
                   "disconnecting and deleting tasks...");
    
    for (const String& topic : subscribedTopics) {
        DEBUG_PRINTF("Unsubscribing from topic: %s\n", topic.c_str());
        client.unsubscribe(topic.c_str());
    }
    
    client.disconnect();
    vTaskDelete(loopTaskHandle);
    instanceCount--;

};

bool MQTTClient::connect(const char certPem[], const char privateKey[], const char clientId[]) {

  DEBUG_PRINTLN("Configuring MQTT client instance");

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(certPem);
  net.setPrivateKey(privateKey);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  //client.begin(AWS_IOT_ENDPOINT, 8883, net);

  client.setServer(MqttEndpoints::awsIoTCoreEndpoint.c_str(), 8883);
  client.setCallback(m_onMqttEvent);

  DEBUG_PRINTF("Connecting to AWS IoT Core endpoint with cliend id: %s\n", clientId);

  while (!client.connect(clientId)) {
    DEBUG_PRINT(client.state());
    DEBUG_PRINT(".");
    delay(100);
  }

  if (!client.connected()) {
    DEBUG_PRINTLN("AWS IoT endpoint timed out. Exiting...");
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

  if (client.subscribe(topic.c_str())) {
      DEBUG_PRINTF("Subscribed to topic: %s\n", topic.c_str());
      subscribedTopics.push_back(topic); // Store for later unsubscription
  } else {
      DEBUG_PRINTF("Failed to subscribe to topic: %s\n", topic.c_str());
  }

};

void MQTTClient::loopTask(void *pvParameters) {

  MQTTClient *instance = static_cast<MQTTClient*>(pvParameters);
  if (instance->client.connected()) instance->client.loop();
  vTaskDelay(pdMS_TO_TICKS(1000));

};

int MQTTClient::instanceCount = 0;