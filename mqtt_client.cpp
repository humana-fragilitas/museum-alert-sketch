#include "mqtt_client.h"

int MQTTClient::instanceCount = 0;

MQTTClient::MQTTClient(std::function<void(const char[], byte*, unsigned int)> onMqttEvent)
    : m_onMqttEvent{onMqttEvent}, net{}, client{net} {

  instanceCount++;
  DEBUG_PRINTF("MQTTClient instance n°: %d\n", instanceCount);

}

MQTTClient::~MQTTClient() {
  
  DEBUG_PRINTLN("MQTTClient Destructor: Unsubscribing, disconnecting, and deleting task...");

  for (const auto& topic : subscribedTopics) {
    client.unsubscribe(topic.c_str());
    DEBUG_PRINTF("Unsubscribing from topic: %s\n", topic.c_str());
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
    m_clientId = clientId;
    m_certPem = certPem;     // Store for reconnection
    m_privateKey = privateKey; // Store for reconnection

    DEBUG_PRINTLN("Configuring MQTT client instance");

    // Always reset the network client (important for reconnections)
    net.stop();
    vTaskDelay(pdMS_TO_TICKS(500)); // Small delay for cleanup

    net.setCACert(AWS_CERT_CA);
    net.setCertificate(certPem.c_str());
    net.setPrivateKey(privateKey.c_str());
    net.setTimeout(10000);

    client.setServer(MqttEndpoints::AWS_IOT_CORE_ENDPOINT, 8883);
    client.setBufferSize(10000);
    client.setCallback(m_onMqttEvent);
    client.setSocketTimeout(10);

    DEBUG_PRINTF("Connecting to AWS IoT Core endpoint with ID: %s\n", clientId.c_str());

    if (client.connect(clientId.c_str())) {
        DEBUG_PRINTLN("MQTT pubsub client successfully connected to AWS IoT Core!");

        // Only create task on first connection
        if (!loopTaskHandle) {
            char taskName[30];
            snprintf(taskName, sizeof(taskName), "MQTT_CLIENT_LOOP_TASK_%d", instanceCount);
            xTaskCreate(
                MQTTClient::loopTaskWrapper,
                taskName, 
                8192,  // Increased stack size
                this,
                instanceCount, 
                &loopTaskHandle
            );
        }

        return true;
    }

    DEBUG_PRINTF("MQTT connection failed, state: %d\n", client.state());
    return false;
}

bool MQTTClient::publish(const char topic[], const char json[]) {

    // Measure length of JSON payload
    size_t length = strlen(json);  // Get the actual size in bytes

    // Convert json (char array) to byte array (uint8_t)
    const uint8_t* payload = reinterpret_cast<const uint8_t*>(json);

    bool success = client.publish(topic, payload, length);

    if (success) {
      DEBUG_PRINTLN("MQTT client successful publish:");
    } else {
      DEBUG_PRINTLN("MQTT client unsuccessful publish:");
    }

    DEBUG_PRINTF("Topic: %s\n", topic);
    DEBUG_PRINTF("Message: %s\n", json);

    return success;

}

void MQTTClient::subscribe(const char topic[]) {

    try {
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
    } catch (String error) {
      DEBUG_PRINTF("[EXCEPTION]: %s\n", error);
    }
}

void MQTTClient::loopTaskWrapper(void* pvParameters) {
  MQTTClient* instance = static_cast<MQTTClient*>(pvParameters);
  instance->loopTask();
}

void MQTTClient::loopTask() {
    while (true) {
        if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000))) {
            DEBUG_PRINTLN("MQTTClient: Task notified to exit.");
            break;
        }
        
        if (!WiFiManager::isConnected()) {
            DEBUG_PRINTLN("MQTTClient: WiFi not connected, waiting...");
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }
        
        if (!client.connected()) {
            DEBUG_PRINTLN("MQTTClient: MQTT disconnected, attempting reconnect...");
            
            // Wait for network to stabilize after WiFi reconnection
            // vTaskDelay(pdMS_TO_TICKS(2000));
            
            // // Completely reset the network client
            // net.stop();
            // vTaskDelay(pdMS_TO_TICKS(500));
            
            // // Re-establish secure connection
            // net.setCACert(AWS_CERT_CA);
            // net.setCertificate(m_certPem.c_str());
            // net.setPrivateKey(m_privateKey.c_str());
            
            // // Add timeouts to prevent hanging
            // net.setTimeout(10000); // 10 second timeout
            // client.setSocketTimeout(10);
            
            DEBUG_PRINTF("Connecting to AWS IoT Core endpoint with ID: %s\n", m_clientId.c_str());
            
            if (MQTTClient::connect(m_certPem, m_privateKey, m_clientId)) {
                DEBUG_PRINTLN("MQTTClient: MQTT reconnected successfully");
                // Re-subscribe to topics
                for (const auto& topic : subscribedTopics) {
                    if (client.subscribe(topic.c_str())) {
                        DEBUG_PRINTF("Re-subscribed to: %s\n", topic.c_str());
                    }
                }
            } else {
                int state = client.state();
                DEBUG_PRINTF("MQTTClient: MQTT reconnect failed, state: %d\n", state);
                vTaskDelay(pdMS_TO_TICKS(5000));
                continue;
            }
        }
        
        client.loop();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
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
