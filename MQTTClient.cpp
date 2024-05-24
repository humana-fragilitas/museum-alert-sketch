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

void MQTTClient::initializeIoTHubClient() {

  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.user_agent = AZ_SPAN_FROM_STR(AZURE_SDK_CLIENT_USER_AGENT);

  if (az_result_failed(az_iot_hub_client_init(
          &client,
          az_span_create((uint8_t*)host, strlen(host)),
          az_span_create((uint8_t*)device_id, strlen(device_id)),
          &options)))
  {
    Serial.println("Failed initializing Azure IoT Hub client");
    return;
  }

  size_t client_id_length;
  if (az_result_failed(az_iot_hub_client_get_client_id(
          &client, mqtt_client_id, sizeof(mqtt_client_id) - 1, &client_id_length)))
  {
    Serial.println("Failed getting client id");
    return;
  }

  if (az_result_failed(az_iot_hub_client_get_user_name(
          &client, mqtt_username, sizeofarray(mqtt_username), NULL)))
  {
    Serial.println("Failed to get MQTT clientId, return code");
    return;
  }

  Serial.println("Client ID: " + String(mqtt_client_id));
  Serial.println("Username: " + String(mqtt_username));
  
}
  
std::pair<esp_mqtt_client_handle_t, int> MQTTClient::initializeMqttClient() {

  unsigned int mqttClientStatus;

  esp_mqtt_client_config_t mqtt_config;
  memset(&mqtt_config, 0, sizeof(mqtt_config));
  mqtt_config.uri = mqtt_broker_uri;
  mqtt_config.port = mqtt_port;
  mqtt_config.client_id = mqtt_client_id;
  mqtt_config.username = mqtt_username;

  Serial.println("MQTT client using X509 Certificate authentication");
  mqtt_config.client_cert_pem = IOT_CONFIG_DEVICE_CERT;
  mqtt_config.client_key_pem = IOT_CONFIG_DEVICE_CERT_PRIVATE_KEY;

  mqtt_config.keepalive = 30;
  mqtt_config.disable_clean_session = 0;
  mqtt_config.disable_auto_reconnect = false;
  mqtt_config.event_handle = _onMqttEvent;
  mqtt_config.user_context = NULL;
  mqtt_config.cert_pem = (const char*)ca_pem;

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

      MQTTClient::initializeIoTHubClient();
      return MQTTClient::initializeMqttClient();

};