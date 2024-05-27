#include <Arduino.h>

// C99 libraries
#include <cstdlib>
#include <string.h>
#include <time.h>

// Libraries for MQTT client and WiFi connection
#include <mqtt_client.h>

// Azure IoT SDK for C includes
#include <az_core.h>
#include <az_iot.h>
#include <azure_ca.h>

#include "MQTTClient.h"

MQTTClient::MQTTClient(esp_err_t(*onMqttEvent)(esp_mqtt_event_handle_t)) {

  _onMqttEvent = onMqttEvent;

}
  
std::pair<esp_mqtt_client_handle_t, int> MQTTClient::initializeMqttClient() {

  unsigned int mqttClientStatus;

  esp_mqtt_client_config_t mqtt_config;
  memset(&mqtt_config, 0, sizeof(mqtt_config));
  mqtt_config.uri = "mqtts://museum-alert-event-grid.westeurope-1.ts.eventgrid.azure.net:8883";
  //mqtt_config.port = 8883;
  mqtt_config.client_id = "MAS-EC357A188534";
  mqtt_config.username = "MAS-EC357A188534";

  Serial.println("MQTT client using X509 Certificate authentication");
  mqtt_config.client_cert_pem = IOT_CONFIG_DEVICE_CERT;
  mqtt_config.client_key_pem = IOT_CONFIG_DEVICE_CERT_PRIVATE_KEY;

  mqtt_config.keepalive = 30;
  mqtt_config.disable_clean_session = 0;
  mqtt_config.disable_auto_reconnect = false;
  mqtt_config.event_handle = _onMqttEvent;
  mqtt_config.user_context = NULL;
  mqtt_config.cert_pem = ROOT;

  mqtt_client = esp_mqtt_client_init(&mqtt_config);

  if (mqtt_client == NULL)
  {
    Serial.println("Failed creating mqtt client");
    mqttClientStatus = 1;
  }

  esp_err_t start_result = esp_mqtt_client_start(mqtt_client);

  if (start_result != ESP_OK)
  {
    Serial.println("Could not start mqtt client; error code:" + start_result);
    mqttClientStatus = 1;
  }
  else
  {
    Serial.println("MQTT client started");
    mqttClientStatus = 0;
  }

  return std::make_pair(mqtt_client, mqttClientStatus);

}

std::pair<esp_mqtt_client_handle_t, int> MQTTClient::connect() {

      return MQTTClient::initializeMqttClient();

};