#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include "MQTTClient.h"

//MQTTClient::MQTTClient(void(*onMqttEvent)(const char[], byte*, unsigned int)) {

MQTTClient::MQTTClient(std::function<void(const char[], byte*, unsigned int)> onMqttEvent) :
    m_onMqttEvent{onMqttEvent}, net{}, client{net} { }

MQTTClient::~MQTTClient() {

    Serial.println("MQTTClient Destructor: unsubscribing from topics and disconnecting...");
    
    for (const String& topic : subscribedTopics) {
        Serial.printf("Unsubscribing from topic: %s\n", topic.c_str());
        client.unsubscribe(topic.c_str());
    }
    
    client.disconnect();

}

bool MQTTClient::connect(const char certPem[], const char privateKey[], const char clientId[]) {

  Serial.println("Configuring MQTT client");

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(certPem);
  net.setPrivateKey(privateKey);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  //client.begin(AWS_IOT_ENDPOINT, 8883, net);

  client.setServer(MqttEndpoints::awsIoTCoreEndpoint.c_str(), 8883);
  client.setCallback(m_onMqttEvent);

  Serial.printf("Connecting to AWS IoT Core endpoint with cliend id: %s\n", clientId);

  while (!client.connect(clientId)) {
    Serial.print(client.state());
    Serial.print(".");
    delay(100);
  }

  if(!client.connected()){
    Serial.println("AWS IoT endpoint timed out. Exiting...");
    return false;
  }

  Serial.println("AWS IoT Core connected!");

  return true;

};

bool MQTTClient::publish(const char topic[], const char json[]) {

  // return client.publish("MAS-EC357A188534/pub", json);
  return client.publish(topic, json);
  
}

void MQTTClient::subscribe(const String& topic) {

  if (client.subscribe(topic.c_str())) {
        Serial.printf("Subscribed to topic: %s\n", topic.c_str());
        subscribedTopics.push_back(topic); // Store for later unsubscription
    } else {
        Serial.printf("Failed to subscribe to topic: %s\n", topic.c_str());
    }

}