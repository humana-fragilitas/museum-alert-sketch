//#include "mbed.h"

#include <mqtt_client.h>
#include <esp_tls.h>
#include <esp_heap_caps.h>

#include "helpers.h"
#include "Pins.h"
#include "PinSetup.h"
#include "SerialCom.h"
#include "Configuration.h"
#include "CertManager.h"
#include "Provisioning.h"
#include "Sensor.h"
#include "WiFiManager.h"
#include "BLEManager.h"
#include "MQTTClient.h"

// Client ID: MAS-EC357A188534

enum AppState {
  STARTED,
  INITIALIZE_BLE,
  CONFIGURE_DEVICE,
  CONNECT_TO_WIFI,
  PROVISION_DEVICE,
  CONNECT_TO_MQTT_BROKER,
  DEVICE_INITIALIZED
};

// Mutex mutex;
TaskHandle_t ledIndicatorsTask;

bool wiFiLedStatus = false;
bool hasBLEConfiguration = false;

AppState appState,lastAppState;

//Configuration configuration;
/*std::pair<WiFiCredentials, ConnectionSettings>*/

BLEManager bleManager;
CertManager certManager;
MQTTClient mqttClient(&onMqttEvent);
ProvisioningSettings provisioningSettings;
WiFiManager wiFiManager(&onWiFiEvent);

unsigned const int configureWiFiInterval = 4000;
unsigned const int sensorInterval = 1000;
unsigned int previousWiFiInterval = 0;
unsigned const int resetButtonInterval = 4000;
unsigned int previousResetButtonInterval = 0;

void setup() {

  initializeSerial();

  pinSetup();

  forceDelay();

  initializeUI();

  lastAppState = STARTED;

  appState = (wiFiManager.connectToWiFi() == WL_CONNECTED) ?
    CONNECT_TO_MQTT_BROKER : INITIALIZE_BLE;

  //BaseType_t coreID = xPortGetCoreID();
  //Serial.print("setup() is running on core ");
  //Serial.println(coreID);

}

void loop() {

  //BaseType_t coreID = xPortGetCoreID();
  //Serial.print("loop() is running on core ");
  //Serial.println(coreID);

  unsigned long currentMillis = millis();

  onEveryMS(currentMillis, 10000, []{
    Serial.println("Free heap memory: ");
    Serial.print(esp_get_free_heap_size()); // Prints available heap in bytes
  });

  switch(appState) {

    case INITIALIZE_BLE:

      onAppStateChange([]{

        Serial.println("Initializing BLE services...");
        if (bleManager.initializeDeviceConfigurationService()) {
          appState = CONFIGURE_DEVICE;
        }

      });

      break;

    case CONFIGURE_DEVICE:

      onAppStateChange([]{
        wiFiManager.disconnect();
        Serial.println("Configuring WiFi...");
      });

      onEveryMS(currentMillis, configureWiFiInterval, []{

        String json = wiFiManager.listNetworks();
        provisioningSettings = bleManager.getDeviceConfiguration(json);

        if (provisioningSettings.isValid()) {
          appState = CONNECT_TO_WIFI;
        } else {
          Serial.println("Received invalid provisioning settings; please resend.");
        }

      });

      break;

    case CONNECT_TO_WIFI:

      onAppStateChange([]{

        Serial.println("Connecting to WiFi...");

        if (wiFiManager.connectToWiFi(
            provisioningSettings.wiFiCredentials.ssid,
            provisioningSettings.wiFiCredentials.password) == WL_CONNECTED) {

        Serial.printf("\nConnected to WiFi network: %s",
          provisioningSettings.wiFiCredentials.ssid.c_str());
        appState = PROVISION_DEVICE;

        } else {
          
          Serial.println("Failed to connect to WiFi network.");
          appState = INITIALIZE_BLE;
          
        }

      });

      break;

    case PROVISION_DEVICE:

      onAppStateChange([]{

        Serial.println("Provisioning device...");

        Provisioning provisiong([=](bool success){
          if (success) {
            certManager.storeCertificates(provisioningSettings.certificates);
            appState = CONNECT_TO_MQTT_BROKER;
          } else {
            appState = INITIALIZE_BLE;
          }
          provisioningSettings.clear();
        });

        if (provisioningSettings.isValid()) {
          provisiong.registerDevice(provisioningSettings.certificates); // TO DO: add callback here: std::function<(bool)> onRegistrationResult; based on the callback argument, set appState
        } else {
          Serial.println("Cannot provision device.");
          appState = INITIALIZE_BLE; 
        }

      });

      break;

    case CONNECT_TO_MQTT_BROKER:

      onAppStateChange([]{

        Certificates certificates;
        bool isConnected;
        
        certificates = certManager.retrieveCertificates();

        if (!certificates.isValid()) {
          appState = INITIALIZE_BLE;
          return;
        }
        
        Serial.println("Connecting to MQTT broker...");

        isConnected = mqttClient.connect(
          certificates.clientCert.c_str(),
          certificates.privateKey.c_str(),
          Sensor::sensorName.c_str()
        );

        appState = isConnected ? DEVICE_INITIALIZED : INITIALIZE_BLE;

      });

      break;

    case DEVICE_INITIALIZED:

      onAppStateChange([]{
        Serial.println("Device initialized.");
        // con
      });

      onEveryMS(currentMillis, sensorInterval, []{

        char outgoingDataTopic[128];

        snprintf(
          outgoingDataTopic,
          sizeof(outgoingDataTopic),
          MqttEndpoints::deviceOutgoingDataTopic.c_str(),
          Sensor::sensorName.c_str()
        );

        Serial.println("Sensor checking for distance...");
        bool hasAlarm = Sensor::detect();
        digitalWrite(alarmPin, hasAlarm);
        if (hasAlarm) {
          if (mqttClient.publish(outgoingDataTopic, (hasAlarm) ? "{ \"alarm\": true, \"distance\": 10 }" : "{ \"alarm\": false }")) {
            Serial.println("Published MQTT message!");
          } else {
            Serial.println("Can't publish MQTT message!");
          }
        }

      });

      break;

  }

}

/******************************************************************************
 * SETUP FUNCTIONS                                                            *
 *****************************************************************************/

void forceDelay() {

  Serial.println("Begin delay: 20 sec.");
  delay(20000);
  Serial.println("Delay end.");

}

void initializeUI() {

  attachInterrupt(digitalPinToInterrupt(resetButtonPin), onResetButtonISR, CHANGE);

  xTaskCreatePinnedToCore(
    ledIndicators,
    "LedIndicators",
    1024,
    NULL,
    0,
    &ledIndicatorsTask,
    0
  );

}

/******************************************************************************
 * LOOP HELPERS                                                               *
 *****************************************************************************/

void onAppStateChange(callback cbFunction) {

  if (appState != lastAppState) {
    lastAppState = appState;
    cbFunction();
  }

}

/******************************************************************************
 * CALLBACK FUNCTIONS                                                         *
 *****************************************************************************/

void onWiFiEvent(WiFiEvent_t event) {

  switch (event) {

    case ARDUINO_EVENT_WIFI_READY: 
        Serial.println("WiFi interface ready");
        digitalWrite(wiFiPin, LOW);
        break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
        Serial.println("Completed scan for access points");
        digitalWrite(wiFiPin, LOW);
        break;
    case ARDUINO_EVENT_WIFI_STA_START:
        Serial.println("WiFi client started");
        digitalWrite(wiFiPin, LOW);
        break;
    case ARDUINO_EVENT_WIFI_STA_STOP:
        Serial.println("WiFi clients stopped");
        digitalWrite(wiFiPin, LOW);
        break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        Serial.println("Connected to access point");
        digitalWrite(wiFiPin, HIGH);
        appState = CONNECT_TO_MQTT_BROKER;
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        Serial.println("Disconnected from WiFi access point");
        digitalWrite(wiFiPin, LOW);
        break;
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        Serial.println("Authentication mode of access point has changed");
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        Serial.println("Obtained IP address: ");
        Serial.print(WiFi.localIP());
        break;
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        Serial.println("Lost IP address and IP address is reset to 0");
        break;

  }

}

void onMqttEvent(const char topic[], byte* payload, unsigned int length) {

  Serial.println("Received [");
  Serial.println(topic);
  Serial.println("]: ");

  // Convert payload to Arduino String
  String message = String((char*)payload).substring(0, length);
  Serial.println(message);

}

void onResetButtonISR(void) {

  // Note: reset button current is pulled down via 10K OHM resistance

  unsigned long currentMillis = millis();

  if (digitalRead(resetButtonPin)) {

    previousResetButtonInterval = currentMillis;

  } else {

    if (currentMillis - previousResetButtonInterval >= resetButtonInterval) {

      Serial.println("Reset button pressed...");
      Serial.println("Erasing AP settings and rebooting...");
      WiFi.eraseAP();
      ESP.restart();
      //wiFiManager.disconnect(true, false);
      //WiFi.eraseAP();
      // esp_wifi_start();
      // Note: first restart after serial flashing causes puts the board in boot mode:(1,7) (purple led)
      // https://github.com/esp8266/Arduino/issues/1722
      // https://github.com/esp8266/Arduino/issues/1017
      // https://github.com/esp8266/Arduino/issues/1722#issuecomment-321818357


    }

  }

}

/******************************************************************************
 * MULTITHREADING FUNCTIONS                                                   *
 ******************************************************************************/

void ledIndicators(void *parameter) {

  const unsigned long defaultInterval = 500;
  const unsigned long configureWiFiInterval = 250;

  for(;;) {

    unsigned long currentMillis = millis();

    switch(appState) {

      case CONFIGURE_DEVICE:
        onEveryMS(currentMillis, configureWiFiInterval, []{
          digitalWrite(appStatusPin, !digitalRead(appStatusPin));
        });
        break;
      default:
        onEveryMS(currentMillis, defaultInterval, []{
          digitalWrite(appStatusPin, !digitalRead(appStatusPin));
        });

    }
    
  }

}

