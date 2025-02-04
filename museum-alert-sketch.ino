//#include "mbed.h"

/**
 * Enable debugging: arduino-cli compile --fqbn esp32:esp32:esp32 --build-property build.extra_flags=-DDEBUG
 */

#include <mqtt_client.h>
#include <esp_tls.h>
#include <esp_heap_caps.h>

#include "macros.h"
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

BLEManager bleManager;
CertManager certManager;
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
    DEBUG_PRINTF("Free heap memory: %d\n", esp_get_free_heap_size());
  });

  switch(appState) {

    case INITIALIZE_BLE:

      onAppStateChange([]{

        DEBUG_PRINTLN("Initializing BLE services...");

        if (bleManager.initializeDeviceConfigurationService()) {
          appState = CONFIGURE_DEVICE;
        }

      });

      break;

    case CONFIGURE_DEVICE:

      onAppStateChange([]{
        wiFiManager.disconnect();
        DEBUG_PRINTLN("Configuring WiFi...");
      });

      onEveryMS(currentMillis, configureWiFiInterval, []{

        String json = wiFiManager.listNetworks();
        provisioningSettings = bleManager.getDeviceConfiguration(json);

        if (provisioningSettings.isValid()) {
          appState = CONNECT_TO_WIFI;
        } else {
          DEBUG_PRINTLN("Received invalid provisioning settings; please resend.");
        }

      });

      break;

    case CONNECT_TO_WIFI:

      onAppStateChange([]{

        DEBUG_PRINTLN("Connecting to WiFi...");

        if (wiFiManager.connectToWiFi(
            provisioningSettings.wiFiCredentials.ssid,
            provisioningSettings.wiFiCredentials.password) == WL_CONNECTED) {

        Serial.printf("\nConnected to WiFi network: %s",
          provisioningSettings.wiFiCredentials.ssid.c_str());
        appState = PROVISION_DEVICE;

        } else {
          
          DEBUG_PRINTLN("Failed to connect to WiFi network.");
          appState = INITIALIZE_BLE;
          
        }

      });

      break;

    case PROVISION_DEVICE:

      onAppStateChange([]{

        DEBUG_PRINTLN("Provisioning device...");

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
          DEBUG_PRINTLN("Cannot provision device.");
          appState = INITIALIZE_BLE; 
        }

      });

      break;

    case CONNECT_TO_MQTT_BROKER:

      onAppStateChange([]{

        DEBUG_PRINTLN("Connecting device to MQTT broker...");
        
        Certificates certificates = certManager.retrieveCertificates();

        appState = (certificates.isValid() && Sensor::connect(certificates)) ?
          DEVICE_INITIALIZED : INITIALIZE_BLE;

      });

      break;

    case DEVICE_INITIALIZED:

      onAppStateChange([]{

        DEBUG_PRINTLN("Device initialized.");

      });

      onEveryMS(currentMillis, sensorInterval, []{

        AlarmPayload payload = Sensor::detect();
        digitalWrite(alarmPin, payload.hasAlarm);
        Sensor::report(payload);

      });

      break;

  }

}

/******************************************************************************
 * SETUP FUNCTIONS                                                            *
 *****************************************************************************/

void forceDelay() {

  DEBUG_PRINTLN("Begin delay: 20 sec.");
  delay(20000);
  DEBUG_PRINTLN("Delay end.");

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
        DEBUG_PRINTLN("WiFi interface ready");
        digitalWrite(wiFiPin, LOW);
        break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
        DEBUG_PRINTLN("Completed scan for access points");
        digitalWrite(wiFiPin, LOW);
        break;
    case ARDUINO_EVENT_WIFI_STA_START:
        DEBUG_PRINTLN("WiFi client started");
        digitalWrite(wiFiPin, LOW);
        break;
    case ARDUINO_EVENT_WIFI_STA_STOP:
        DEBUG_PRINTLN("WiFi clients stopped");
        digitalWrite(wiFiPin, LOW);
        break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        DEBUG_PRINTLN("Connected to access point");
        digitalWrite(wiFiPin, HIGH);
        appState = CONNECT_TO_MQTT_BROKER;
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        DEBUG_PRINTLN("Disconnected from WiFi access point");
        digitalWrite(wiFiPin, LOW);
        break;
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        DEBUG_PRINTLN("Authentication mode of access point has changed");
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        DEBUG_PRINTLN("Obtained IP address: ");
        Serial.print(WiFi.localIP());
        break;
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        DEBUG_PRINTLN("Lost IP address and IP address is reset to 0");
        break;

  }

}

void onResetButtonISR(void) {

  // Note: reset button current is pulled down via 10K OHM resistance

  unsigned long currentMillis = millis();

  if (digitalRead(resetButtonPin)) {

    previousResetButtonInterval = currentMillis;

  } else {

    if (currentMillis - previousResetButtonInterval >= resetButtonInterval) {

      DEBUG_PRINTLN("Reset button pressed...");
      DEBUG_PRINTLN("Erasing AP settings and rebooting...");
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

