#include "mqtt_client.h"

int MQTTClient::instanceCount = 0;

MQTTClient::MQTTClient(std::function<void(const char[], byte*, unsigned int)> onMqttEvent)
    : m_onMqttEvent{onMqttEvent}, net{}, client{net} {

  instanceCount++;

}

MQTTClient::~MQTTClient() {
  
  DEBUG_PRINTLN("MQTTClient Destructor: Unsubscribing, disconnecting, and deleting task...");

  for (const auto& topic : subscribedTopics) {
    client.unsubscribe(topic.c_str());
    DEBUG_PRINTF("Unsubscribing from topic: %s\n", topic);
  }

  subscribedTopics.clear();
  
  client.disconnect();

  if (loopTaskHandle) {
    xTaskNotifyGive(loopTaskHandle);
    vTaskDelay(pdMS_TO_TICKS(100));
    vTaskDelete(loopTaskHandle);
    loopTaskHandle = nullptr;
  }

  instanceCount--;

}

bool MQTTClient::connect(String certPem, String privateKey, String clientId) {

  bool hasAttemptedConnection = true;
  m_clientId = clientId;

  DEBUG_PRINTLN("Configuring MQTT client instance");

  net.setCACert(AWS_CERT_CA);
  net.setCertificate(certPem.c_str());
  net.setPrivateKey(privateKey.c_str());

  client.setServer(MqttEndpoints::AWS_IOT_CORE_ENDPOINT, 8883);
  client.setBufferSize(10000);
  client.setCallback(m_onMqttEvent);

  DEBUG_PRINTF("Connecting to AWS IoT Core endpoint with ID: %s\n", clientId.c_str());

  if (client.connect(clientId.c_str())) {
      DEBUG_PRINTLN("MQTT pubsub client successfully connected to AWS IoT Core!");

      if (!loopTaskHandle) {

        char taskName[30];
        snprintf(taskName, sizeof(taskName), "MQTT_CLIENT_LOOP_TASK_%d", instanceCount);
        xTaskCreate(
        MQTTClient::loopTaskWrapper,
        taskName, 
        4096, 
        this,
        1, 
        &loopTaskHandle
        );

      }

      return true;
  }

  return false;

}

bool MQTTClient::publish(const char topic[], const char json[]) {

    DEBUG_PRINTLN("MQTT client publish:");
    DEBUG_PRINTF("Topic: %s\n", topic);
    DEBUG_PRINTF("Message: %s\n", json);

    // Measure length of JSON payload
    size_t length = strlen(json);  // Get the actual size in bytes

    // Convert json (char array) to byte array (uint8_t)
    const uint8_t* payload = reinterpret_cast<const uint8_t*>(json);

    return client.publish(topic, payload, length);

}

void MQTTClient::subscribe(const char topic[]) {

    String topicStr(topic);
    auto result = subscribedTopics.insert(topicStr);
    
    if (result.second) {
        if (client.subscribe(topic)) {
            DEBUG_PRINTF("Subscribed to topic: %s\n", topic);
        } else {
            DEBUG_PRINTF("Failed to subscribe to topic: %s\n", topic);
            subscribedTopics.erase(topicStr);
        }
    } else {
        DEBUG_PRINTF("Already subscribed to topic: %s\n", topic);
    }

}

void MQTTClient::loopTaskWrapper(void* pvParameters) {
  MQTTClient* instance = static_cast<MQTTClient*>(pvParameters);
  instance->loopTask();
}

void MQTTClient::loopTask() {

  // while (true) {
  //       // Check if task should exit
  //       if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000))) {
  //           DEBUG_PRINTLN("MQTTClient: Task notified to exit.");
  //           break;
  //       }

  //       // Handle MQTT connection
  //       if (!client.connected() && !hasAttemptedConnection) {
  //           DEBUG_PRINTLN("MQTTClient: Disconnected. Attempting to reconnect...");
  //           if (!client.connect(m_clientId.c_str())) {
  //               DEBUG_PRINTLN("MQTTClient: Reconnect failed. Retrying in 10s...");
  //               vTaskDelay(pdMS_TO_TICKS(10000)); // Sleep before retrying
  //           } else {
  //               DEBUG_PRINTLN("MQTTClient: Successfully reconnected!");
  //           }
  //       }

  //       client.loop();
  //       vTaskDelay(pdMS_TO_TICKS(500)); // Reduce CPU usage
  //   }

  //   DEBUG_PRINTLN("MQTTClient: Exiting loop task.");
  //   loopTaskHandle = NULL;
  //   vTaskDelete(NULL);

  while (client.connected()) {
    if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000))) {
      DEBUG_PRINTLN("MQTTClient: Task notified to exit.");
      break;
    }
    vTaskDelay(pdMS_TO_TICKS(500));
    client.loop();
  }

  // DEBUG_PRINTLN("MQTTClient: Exiting loop task.");
  // vTaskDelete(nullptr);

/*
       // Check if task should exit
        if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000))) {
            DEBUG_PRINTLN("MQTTClient: Task notified to exit.");
            break;
        }

        // Handle MQTT connection
        if (!client.connected()) {
            DEBUG_PRINTLN("MQTTClient: Disconnected. Attempting to reconnect...");
            if (!client.connect("yourClientID")) {
                DEBUG_PRINTLN("MQTTClient: Reconnect failed. Retrying in 10s...");
                vTaskDelay(pdMS_TO_TICKS(10000)); // Sleep before retrying
            } else {
                DEBUG_PRINTLN("MQTTClient: Successfully reconnected!");
            }
        }

        client.loop();
        vTaskDelay(pdMS_TO_TICKS(500)); // Reduce CPU usage
    }

    DEBUG_PRINTLN("MQTTClient: Exiting loop task.");
    mqttTaskHandle = NULL;
    vTaskDelete(NULL);

*/

    // while (true) {
    //     if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000))) {
    //         DEBUG_PRINTLN("MQTTClient: Task notified to exit.");
    //         break;
    //     }

    //     // Handle reconnection logic **only if** connect() was attempted at least once
    //     if (hasAttemptedConnection && !client.connected()) {
    //         DEBUG_PRINTLN("MQTTClient: Disconnected, attempting reconnection...");

    //         if (!client.connect(m_clientId.c_str())) {  // You might want to store the last used ClientID
    //             DEBUG_PRINTLN("MQTTClient: Reconnection failed.");
    //         } else {
    //             DEBUG_PRINTLN("MQTTClient: Reconnected successfully.");
    //         }
    //     }

    //     client.loop();
    //     vTaskDelay(pdMS_TO_TICKS(500)); // Reduce CPU usage
    // }

    // DEBUG_PRINTLN("MQTTClient: Exiting loop task.");
    // vTaskDelete(nullptr);

}

bool MQTTClient::isConnected() {
  return client.connected();
}

// Amazon Root CA 1
const char MQTTClient::AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";
