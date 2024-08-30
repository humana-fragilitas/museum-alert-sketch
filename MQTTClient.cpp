#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include "MQTTClient.h"

MQTTClient::MQTTClient(void(*onMqttEvent)(const char[], byte*, unsigned int)) {

  net = WiFiClientSecure();
  client = PubSubClient(net);

  _onMqttEvent = onMqttEvent;

}

void MQTTClient::connect(const char certPem[], const char privateKey[]) {

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(certPem);
  net.setPrivateKey(privateKey);

  Serial.println("Initializing MQTT Client");

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  //client.begin(AWS_IOT_ENDPOINT, 8883, net);


  client.setServer(AWS_IOT_ENDPOINT, 8883);
  client.setCallback(_onMqttEvent);

  Serial.println("Connecting to AWS");

  while (!client.connect(THINGNAME)) {
    Serial.println(client.state());
    Serial.print(".");
    delay(100);
  }

  if(!client.connected()){
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("AWS IoT Connected!");

};

bool MQTTClient::publish(const char json[]) {

  return client.publish("MAS-EC357A188534/pub", json);

}